/*
 * watchdogDebug.h
 * 
 * Utilidades de debugging y verificación del watchdog
 * Para comprobar que:
 * - El watchdog se ejecuta periódicamente (determinismo)
 * - El mutex protege correctamente lastHeartbeatTick
 * - La soberanía del watchdog se mantiene incluso con MQTT bloqueado
 */

#ifndef WATCHDOG_DEBUG_H
#define WATCHDOG_DEBUG_H

#include <Arduino.h>

// Estadísticas de ejecución del watchdog
struct WatchdogStats {
  uint32_t totalCycles;
  uint32_t timeouts;
  uint32_t safeClosures;
  TickType_t lastExecutionTime;
  uint32_t mutexTimeouts;
};

// Variable global para estadísticas (si se activa en config)
#ifdef DEBUG_WATCHDOG
extern WatchdogStats g_watchdogStats;

#define WATCHDOG_STAT_INC_CYCLES()   (g_watchdogStats.totalCycles++)
#define WATCHDOG_STAT_INC_TIMEOUTS() (g_watchdogStats.timeouts++)
#define WATCHDOG_STAT_INC_CLOSURES() (g_watchdogStats.safeClosures++)
#define WATCHDOG_STAT_INC_MUTEX_TO() (g_watchdogStats.mutexTimeouts++)
#else
#define WATCHDOG_STAT_INC_CYCLES()
#define WATCHDOG_STAT_INC_TIMEOUTS()
#define WATCHDOG_STAT_INC_CLOSURES()
#define WATCHDOG_STAT_INC_MUTEX_TO()
#endif

#endif // WATCHDOG_DEBUG_H
