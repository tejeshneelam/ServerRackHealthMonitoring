import streamlit as st
import json
import os
import plotly.graph_objects as go
from digital_twin_math import generate_3d_thermal_map

st.set_page_config(page_title="AI Data Center Monitor", layout="wide", page_icon="🖥️")

# Custom CSS for modern dark-mode dashboard feel without page flicker
st.markdown("""
<style>
    .metric-card {
        background-color: #1e1e1e;
        border-radius: 10px;
        padding: 20px;
        box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        text-align: center;
        border: 1px solid #333;
    }
    .status-healthy { color: #00ff00; font-weight: bold; font-size: 22px; }
    .status-warning { color: #ffff00; font-weight: bold; font-size: 22px; }
    .status-danger { color: #ff0000; font-weight: bold; font-size: 22px; }
    .relay-on { color: #00e5ff; font-weight: bold; font-size: 18px; }
    .relay-off { color: #888888; font-weight: bold; font-size: 18px; }
</style>
""", unsafe_allow_html=True)

DATA_FILE = os.path.join(os.path.dirname(__file__), 'latest_data.json')

def load_data():
    try:
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, 'r') as f:
                return json.load(f)
    except:
        pass
    return {
        "t1": 25.0, "t2": 25.0, "t3": 25.0, "t4": 25.0,
        "ambT": 25.0, "ambH": 50.0,
        "gas": 0, "gasVolt": 0.0,
        "vibe": 0, "vibeCount": 0
    }

# Main Static Header (Renders once, never flickers)
st.title("🖥️ AI-Powered Data Center Rack Health Monitor")
st.markdown("### PI-SSET & E-CSGI Edge-Server Dashboard")

# High-Performance Zero-Flicker WebGL 3D Digital Twin Dashboard
# The embedded client-side architecture maintains 100% persistent camera rotation/zoom
st.components.v1.iframe("http://localhost:8000", height=900, scrolling=True)
