//#pragma message("!!! In tivacan.h ")

#ifndef AT90_CAN_H
#define AT90_CAN_H

//#pragma message("!!! compiling AT90can.h ")

#if defined (__cplusplus)
extern "C" {
#endif
    
// #include <avr/pgmspace.h>
// #include <stdint.h>
// #include <stdbool.h>
#include "CanBus.h"
    

//#define	SUPPORT_TIMESTAMPS
    
    typedef struct
    {
        uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
        struct {
            int rtr : 1;			//!< Remote-Transmit-Request-Frame?
            int extended : 1;		//!< extended ID?
        } flags;
        uint8_t length;				//!< Anzahl der Datenbytes
        uint8_t data[8];			//!< Die Daten der CAN Nachricht
         #if SUPPORT_TIMESTAMPS
          uint16_t timestamp;
         #endif
    } tCAN;
//#pragma message("!!! AT90can.h tCAN")

    typedef struct
    {
        uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
        uint32_t mask;				//!< Maske
        struct {
            uint8_t rtr : 2;		//!< Remote Request Frame
            uint8_t extended : 2;	//!< extended ID
        } flags;
    } tCANFilter;
    
    typedef struct {
        uint8_t rx;				//!< Empfangs-Register
        uint8_t tx;				//!< Sende-Register
    } tCANErrorRegister;
    
    typedef enum {
        eLISTEN_ONLY_MODE,		//!< der CAN Contoller empfängt nur und verhält sich völlig passiv
        eLOOPBACK_MODE,			//!< alle Nachrichten direkt auf die Empfangsregister umleiten ohne sie zu senden
        eNORMAL_MODE				//!< normaler Modus, CAN Controller ist aktiv
    } tCANMode;
    

    extern bool can_init(uint8_t bitrate);
    extern bool can_set_filter(uint8_t number, const tCANFilter *filter);
    extern bool can_disable_filter(uint8_t number);
    extern void can_static_filter(const uint8_t *filter_array);
    extern uint8_t can_get_filter(uint8_t number, tCANFilter *filter);

    extern bool can_check_message(void);
    extern bool can_check_free_buffer(void);
    extern uint8_t can_send_message(const tCAN *msg);
    extern uint8_t can_get_message(tCAN *msg);

    extern tCANErrorRegister can_read_error_register(void);
    extern bool can_check_bus_off(void);
    extern void can_reset_bus_off(void);
    extern void can_set_mode(tCANMode mode);
    extern void can_regdump(void);
    
#if defined (__cplusplus)
}
#endif

#endif // CAN_H
