from fastapi import FastAPI, Header, HTTPException
from pydantic import BaseModel
from typing import List, Optional
import sqlite3
import json
import os

app = FastAPI()

TOKEN_SECRETO = os.getenv("API_TOKEN")
if not TOKEN_SECRETO:
    raise RuntimeError("ERRO FATAL: Variável de ambiente 'API_TOKEN' não configurada. Abortando por segurança.")

DB_FILE = "data/database.sqlite"

# --- Setup do SQLite ---
def init_db():
    os.makedirs(os.path.dirname(DB_FILE), exist_ok=True)
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS signals (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT,
            type INTEGER,
            freq REAL,
            decodedValue INTEGER,
            bitlength INTEGER,
            rawDurations TEXT,
            rawCount INTEGER,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    conn.commit()
    conn.close()

init_db()
# -----------------------

class SavedSignal(BaseModel):
    name: Optional[str] = "Captura Desconhecida"
    type: int
    freq: float
    decodedValue: Optional[int] = 0
    bitlength: Optional[int] = 0
    rawDurations: Optional[List[int]] = []
    rawCount: Optional[int] = 0

@app.post("/rf/salvar")
def salvar_sinal(sinal: SavedSignal, authorization: str = Header(None)):
    if authorization != f"Bearer {TOKEN_SECRETO}":
        raise HTTPException(status_code=401, detail="Não autorizado")
    
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    
    # Converte a lista de pulsos brutos para uma string JSON para salvar numa coluna TEXT
    raw_durations_json = json.dumps(sinal.rawDurations)
    
    cursor.execute('''
        INSERT INTO signals (name, type, freq, decodedValue, bitlength, rawDurations, rawCount)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    ''', (
        sinal.name, 
        sinal.type, 
        sinal.freq, 
        sinal.decodedValue, 
        sinal.bitlength, 
        raw_durations_json, 
        sinal.rawCount
    ))
    
    conn.commit()
    conn.close()
    
    return {"status": "ok", "message": "Sinal guardado no banco SQLite!"}

@app.get("/rf/historico")
def ler_historico(authorization: str = Header(None), limit: int = 50):
    if authorization != f"Bearer {TOKEN_SECRETO}":
        raise HTTPException(status_code=401, detail="Não autorizado")
    
    conn = sqlite3.connect(DB_FILE)
    conn.row_factory = sqlite3.Row # Para retornar dicionários ao invés de tuplas numéricas
    cursor = conn.cursor()
    
    cursor.execute('SELECT * FROM signals ORDER BY id DESC LIMIT ?', (limit,))
    rows = cursor.fetchall()
    conn.close()
    
    resultados = []
    for row in rows:
        dict_row = dict(row)
        # Transforma o JSON de volta para Lista de inteiros
        dict_row['rawDurations'] = json.loads(dict_row['rawDurations'])
        resultados.append(dict_row)
        
    return resultados