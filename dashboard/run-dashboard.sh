#!/usr/bin/env bash
# Script mínimo para servir la carpeta `dashboard` con Python 3
# Uso: ./run-dashboard.sh [port]

PORT=${1:-8000}
ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

cd "$ROOT_DIR" || exit 1

echo "Sirviendo $ROOT_DIR en http://localhost:$PORT"
echo "Presiona Ctrl+C para detener el servidor."

# Iniciar el servidor en background y guardar PID
python3 -m http.server "$PORT" &
PID=$!

# Asegurar que el servidor se cierre cuando el script reciba SIGINT/SIGTERM
trap 'kill ${PID} 2>/dev/null || true; exit' INT TERM EXIT

# Esperar un momento para que el servidor inicie completamente
sleep 1

# Abrir la página professional.html en el navegador
URL="http://localhost:$PORT/professional.html"
echo "Abriendo $URL en el navegador..."

# Intentar diferentes métodos de apertura de navegador
opened=false

if command -v xdg-open >/dev/null 2>&1; then
	# Linux estándar - usar nohup para evitar que bloquee
	nohup xdg-open "$URL" >/dev/null 2>&1 &
	opened=true
elif command -v gnome-open >/dev/null 2>&1; then
	# GNOME
	nohup gnome-open "$URL" >/dev/null 2>&1 &
	opened=true
elif command -v kde-open >/dev/null 2>&1; then
	# KDE
	nohup kde-open "$URL" >/dev/null 2>&1 &
	opened=true
elif command -v open >/dev/null 2>&1; then
	# macOS
	open "$URL" >/dev/null 2>&1 &
	opened=true
elif command -v wslview >/dev/null 2>&1; then
	# WSL
	wslview "$URL" >/dev/null 2>&1 &
	opened=true
elif command -v sensible-browser >/dev/null 2>&1; then
	# Debian/Ubuntu
	sensible-browser "$URL" >/dev/null 2>&1 &
	opened=true
fi

if [ "$opened" = false ]; then
	echo "⚠️  No se pudo detectar navegador. Abre manualmente: $URL"
else
	echo "✓ Navegador abierto con $URL"
fi

# Esperar al proceso del servidor (mantiene script activo y permite Ctrl+C para detener)
wait ${PID}

# Limpiar trap
trap - INT TERM EXIT

