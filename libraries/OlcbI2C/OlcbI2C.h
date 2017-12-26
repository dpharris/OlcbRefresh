//#pragma message("!!! In OlcbI2C.h ")

//#ifndef CAN_H
//#define CAN_H

#ifndef OlcbI2C_h
#define OlcbI2C_h

//#pragma message("!!! compiling OlcbI2C.h ")

#if defined (__cplusplus)
extern "C" {
#endif
    
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>
    
    
    //#define  SUPPORT_TIMESTAMPS
    
    typedef struct
    {
        uint32_t id;        //!< ID der Nachricht (11 oder 29 Bit)
        struct {
            int rtr : 1;      //!< Remote-Transmit-Request-Frame?
            int extended : 1;   //!< extended ID?
        } flags;
        uint8_t length;       //!< Anzahl der Datenbytes
        uint8_t data[8];      //!< Die Daten der CAN Nachricht
#if SUPPORT_TIMESTAMPS
        uint16_t timestamp;
#endif
    } tCAN;
    //#pragma message("!!! tivacan.h tCAN")
    
    typedef struct
    {
        uint32_t id;        //!< ID der Nachricht (11 oder 29 Bit)
        uint32_t mask;        //!< Maske
        struct {
            uint8_t rtr : 2;    //!< Remote Request Frame
            uint8_t extended : 2; //!< extended ID
        } flags;
    } tCANFilter;
    
    typedef struct {
        uint8_t rx;       //!< Empfangs-Register
        uint8_t tx;       //!< Sende-Register
    } tCANErrorRegister;
    
    typedef enum {
        LISTEN_ONLY_MODE,   //!< der CAN Contoller empfängt nur und verhält sich völlig passiv
        LOOPBACK_MODE,      //!< alle Nachrichten direkt auf die Empfangsregister umleiten ohne sie zu senden
        NORMAL_MODE       //!< normaler Modus, CAN Controller ist aktiv
    } tCANMode;
    
    typedef struct {
        uint32_t id;
        uint8_t length;
        uint8_t data[8];
    } I2C_t;
    
    extern bool can_init(uint8_t bitrate){ Wire.begin(); }
    extern bool can_set_filter(uint8_t number, const tCANFilter *filter);
    extern bool can_disable_filter(uint8_t number);
    extern void can_static_filter(const uint8_t *filter_array);
    extern uint8_t can_get_filter(uint8_t number, tCANFilter *filter);
    
    extern bool can_check_message(void) { return Wire.available(); }
    extern bool can_check_free_buffer(void) { return true; }
    extern uint8_t can_send_message(const tCAN *msg)
    {
        I2C_t buf;
        buf.id = msg->id;
        buf.length = msg->length;
        for(unsigned i=0;i<buf.length;i++) buf.data[i]=msg->data[i];
        Wire.beginTransmission(0);
        Wire.write((uint8_t*)msg, buf.length+5);
        Wire.endTransmission();
    }
    extern uint8_t can_get_message(tCAN *msg)
    {
        I2C_t buf;
        uint8_t* pbuf;
        uint8_t n = 0;
        while(Wire.available()) {
            *pbuf++ = Wire.read();
            n++;
            if(n>5 && n>=(5+buf.length)) break;
        }
        msg->id = buf.id;
        msg->length = buf.length;
        for(unsigned i=0;i<buf.length;i++) msg->data[i]=buf.data[i];
        return n;
    }
    
    extern tCANErrorRegister can_read_error_register(void);
    extern bool can_check_bus_off(void);
    extern void can_reset_bus_off(void);
    extern void can_set_mode(tCANMode mode);
    extern void can_regdump(void);
    
#if defined (__cplusplus)
}
#endif

#endif // CAN_H
