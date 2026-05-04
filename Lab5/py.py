import streamlit as st
import pandas as pd
import numpy as np
import glob
import os
from io import StringIO

# =========================
# КОНФІГУРАЦІЯ
# =========================

st.set_page_config(
    page_title="VHI Аналіз України",
    page_icon="🌿",
    layout="wide"
)

NOAA_TO_UA_NAME = {
    1:"Черкаська",2:"Чернігівська",3:"Чернівецька",4:"Кримська",
    5:"Дніпропетровська",6:"Донецька",7:"Івано-Франківська",
    8:"Харківська",9:"Херсонська",10:"Хмельницька",11:"Київська",
    12:"Київська міська",13:"Кіровоградська",14:"Луганська",
    15:"Львівська",16:"Миколаївська",17:"Одеська",18:"Полтавська",
    19:"Рівненська",20:"Севастополь",21:"Сумська",22:"Тернопільська",
    23:"Закарпатська",24:"Вінницька",25:"Волинська"
}

# =========================
# ЗАВАНТАЖЕННЯ ДАНИХ
# =========================

@st.cache_data
def load_data(path="vhi_data"):
    files = glob.glob(os.path.join(path, "*.csv"))
    if not files:
        return None

    dfs = []
    for file in files:
        try:
            with open(file, "r", encoding="utf-8") as f:
                lines = f.readlines()

            clean = [l.replace('<tt><pre>', '').replace('<br>', '').strip().rstrip(',') for l in lines if l.strip()]
            text = "\n".join(clean[1:])
            df = pd.read_csv(StringIO(text))

            df = df[pd.to_numeric(df['year'], errors='coerce').notna()]
            df['year'] = df['year'].astype(int)
            df['week'] = df['week'].astype(int)
            df['VHI'] = pd.to_numeric(df['VHI'], errors='coerce')
            df = df[(df['VHI'] != -1) & df['VHI'].notna()]

            rid = int(os.path.basename(file).split('_')[2])
            df["region"] = NOAA_TO_UA_NAME.get(rid, f"Область {rid}")

            dfs.append(df)

        except:
            continue

    return pd.concat(dfs)

df = load_data()

if df is None:
    st.error("❌ Немає файлів у папці vhi_data")
    st.stop()

# =========================
# SIDEBAR
# =========================

st.sidebar.title("Фільтри")

index_type = st.sidebar.selectbox("Індекс", ["VCI","TCI","VHI"])
region = st.sidebar.selectbox("Область", df["region"].unique())

week_range = st.sidebar.slider("Тижні", 1, 52, (1,52))
year_range = st.sidebar.slider("Роки", int(df["year"].min()), int(df["year"].max()), (2000,2024))

# =========================
# ФІЛЬТР
# =========================

filtered = df[
    (df["region"] == region) &
    (df["year"].between(year_range[0], year_range[1])) &
    (df["week"].between(week_range[0], week_range[1]))
]

# =========================
# ВКЛАДКИ
# =========================

tab1, tab2, tab3 = st.tabs(["Таблиця","Графік","Порівняння"])

with tab1:
    st.dataframe(filtered)

with tab2:
    st.line_chart(filtered.set_index("year")[index_type])

with tab3:
    compare = df.groupby("region")[index_type].mean()
    st.bar_chart(compare)

st.success("✅ Працює")