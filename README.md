# 🌿 Sistema de Riego Inteligente con Mecanismo *Fail-Safe* (ESP32 + FreeRTOS + MQTT)

> Proyecto académico de **Sistemas de Tiempo Real** – Universidad Tecnológica Nacional  
> Implementación de un sistema de riego remoto, seguro y determinista, basado en **ESP32**, **FreeRTOS** y **MQTT**.

---

## 📘 Descripción General

Este proyecto implementa un sistema de **control remoto de válvula solenoide de riego (12 V)** mediante un **ESP32** conectado a un **servidor MQTT**, incorporando un mecanismo de **fail-safe en tiempo real**.

El principio de funcionamiento se basa en una señal periódica de *heartbeat* enviada desde la base de control.  
Si el nodo actuador (ESP32) no recibe esta señal durante un tiempo predefinido (5 s), un **watchdog** toma el control y **cierra automáticamente la válvula**, garantizando un estado seguro.

---

## 🎯 Objetivos

### Objetivo principal
- Garantizar un **control seguro, determinista y confiable** del sistema de riego remoto.

### Objetivos específicos
- Configurar un **broker MQTT** para la comunicación entre el servidor y el nodo remoto.  
- Implementar **FreeRTOS en el ESP32** con una jerarquía de tareas bien definida:
  - **Tarea crítica (Watchdog):** monitoreo del heartbeat y cierre de la válvula ante pérdida de señal.
  - **Tarea media (Cliente MQTT):** recepción de comandos y reinicio del temporizador del watchdog.
- Diseñar un **dashboard web** para el monitoreo remoto y envío de comandos.

---

## ⚙️ Arquitectura del Sistema

### Nodo Base (Servidor MQTT)
- Envía *heartbeats* y comandos de control (`ABRIR`, `CERRAR`).
- Aloja un **dashboard web** de monitoreo.
- Mantiene registro del estado de la válvula.

### Nodo Actuador (ESP32)
- Controla un **módulo relé de 1 canal (5 V)** conectado a una **válvula solenoide de 12 V**.
- Implementa tareas FreeRTOS con distintas prioridades.
- Activa un **modo de bajo consumo** cuando la válvula está cerrada.


---

## 🧠 Software y Tecnologías

- **ESP-IDF / Arduino Framework**
- **FreeRTOS** → planificación de tareas y watchdog.
- **PubSubClient** → comunicación MQTT.
- **HTML / JS / Node.js** → dashboard web.
- **Mosquitto / EMQX** → broker MQTT.

---

## 🧵 Estructura de Tareas (FreeRTOS)

| Tarea | Prioridad | Función |
|-------|------------|---------|
| `taskWatchdog` | Alta | Supervisa heartbeat, controla el cierre seguro |
| `taskMQTT` | Media | Escucha mensajes y publica estado |
| `taskNet` | Baja | Mantiene conexión WiFi y publica datos periódicos |

---

## 🧰 Instalación y Uso

1. **Clonar el repositorio:**
   ```bash
   git clone https://github.com/<usuario>/sistema-riego-esp32.git
   cd sistema-riego-esp32
