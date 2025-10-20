TP-Riego-Automatico 💧
Control remoto de un sistema de riego con mecanismo fail-safe en tiempo real, utilizando un ESP32, FreeRTOS y MQTT.

Este proyecto implementa un sistema de control para una válvula solenoide de riego de 12V. La característica principal es su mecanismo de seguridad (fail-safe), que garantiza que la válvula solo permanezca abierta si recibe una señal de heartbeat (latido) constante desde el servidor. Si la comunicación se pierde durante más de 5 segundos, un watchdog de software toma el control y cierra la válvula automáticamente, previniendo inundaciones o desperdicio de agua.

Tabla de Contenidos
Descripción General

Objetivos del Proyecto

Arquitectura del Firmware (FreeRTOS)

Stack Tecnológico

Instalación

Uso

Descripción General
El núcleo del sistema es un microcontrolador ESP32 que actúa como un nodo remoto. Este nodo se conecta a un servidor MQTT para recibir comandos y enviar su estado.

El principal desafío que resuelve este proyecto es la falta de fiabilidad en los sistemas de riego remotos convencionales. Si el controlador pierde conexión mientras la válvula está abierta, esta podría permanecer así indefinidamente. Nuestro sistema soluciona esto con un watchdog de conexión: el servidor debe enviar un heartbeat periódico para mantener la válvula abierta. Si el ESP32 no recibe este heartbeat en 5 segundos, asume una falla de conexión y entra en estado seguro (válvula cerrada).

Objetivos del Proyecto
Objetivo Principal
Garantizar un control seguro, determinista y confiable de un sistema de riego remoto.

Configurar un servidor MQTT con un canal dedicado para enviar señales de control (ABRIR, CERRAR, HEARTBEAT) al nodo remoto.

Configurar el ESP32 para operar en modo de bajo consumo y escuchar las señales MQTT.

Implementar un sistema multitarea en el ESP32 usando FreeRTOS, definiendo prioridades claras para las tareas críticas de seguridad.

Objetivo Secundario
Implementar una dashboard web para la gestión y monitoreo remoto del sistema.

Arquitectura del Firmware (FreeRTOS)
El sistema operativo en tiempo real (RTOS) es esencial para garantizar que la tarea de seguridad (el watchdog) nunca sea bloqueada por tareas de menor prioridad (como la conexión de red).

Tarea 1 (Prioridad Crítica): Watchdog de Conexión 🚨
Descripción: Un temporizador de alta prioridad que se reinicia con cada señal de heartbeat válida recibida vía MQTT.

Función Fail-Safe: Si este temporizador expira (tras 5 segundos sin señal), la tarea toma control inmediato del relé y lo lleva al estado seguro (cerrar la válvula), independientemente de lo que esté haciendo cualquier otra tarea.

Tarea 2 (Prioridad Media): Cliente MQTT 📡
Descripción: Se encarga de la conectividad y la lógica de negocio.

Funciones:

Conectarse a la red Wi-Fi y al servidor MQTT.

Escuchar los mensajes del servidor (comandos "ABRIR", "CERRAR").

Ejecutar los comandos recibidos para controlar el relé.

Función Crítica: Reiniciar el temporizador del watchdog (Tarea 1) cada vez que se recibe un heartbeat.

Stack Tecnológico
Hardware
Microcontrolador: ESP32

Actuador: Válvula Solenoide de 12V

Interfaz: Módulo Relé (para que el ESP32 controle la válvula de 12V)

Software
Sistema Operativo: FreeRTOS (nativo de ESP-IDF o vía Arduino)

Protocolo de Comunicación: MQTT

Plataforma: ESP-IDF o Arduino Framework

Instalación
(Aquí deberías agregar las instrucciones para clonar e instalar el proyecto)

Bash

# Ejemplo:
git clone https://github.com/tu-usuario/TP-Riego-Automatico.git
cd TP-Riego-Automatico
# ...instrucciones de configuración de entorno, librerías, etc.
Uso
(Aquí deberías explicar cómo usar el sistema)

Configurar las credenciales de Wi-Fi y del broker MQTT en el archivo config.h.

Compilar y flashear el firmware en el ESP32.

Publicar en el topic MQTT riego/control los siguientes mensajes:

"ABRIR": Para abrir la válvula.

"CERRAR": Para cerrar la válvula.

Importante: Se debe enviar un mensaje heartbeat (p.ej. "PING") al topic riego/heartbeat cada 4 segundos (o menos) para mantener la válvula abierta.
