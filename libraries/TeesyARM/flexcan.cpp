// OpenLCB Adaptation of FlexCAN library
// copyright DPH 2017

//#pragma message("!!! compiling flaxcan.cpp ")

#include "flexcan.h"
#include "FlexCANv1.h"

FlexCANv1 fcan(125000);
        
        
// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Initialisierung des CAN Interfaces
 *
 * \param	bitrate	Gewuenschte Geschwindigkeit des CAN Interfaces
 *
 * \return	false falls das CAN Interface nicht initialisiert werden konnte,
 *			true ansonsten.
 */
bool can_init(uint8_t bitrate) {
    fcan.begin();
    return true;
}

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzen eines Filters
 * 
 * Für einen MCP2515 sollte die Funktion can_static_filter() bevorzugt werden.
 *
 * \param	number	Position des Filters
 * \param	filter	zu setzender Filter
 *
 * \return	false falls ein Fehler auftrat, true ansonsten
 */
extern bool can_set_filter(uint8_t number, const tCANFilter *filter);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Filter deaktivieren
 *
 * \param	number	Nummer des Filters der deaktiviert werden soll,
 *			0xff deaktiviert alle Filter.
 * \return	false falls ein Fehler auftrat, true ansonsten
 *
 * \warning Wird nur vom AT90CAN32/64/128 unterstuetzt.
 */
extern bool can_disable_filter(uint8_t number);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzt die Werte für alle Filter
 *
 * \param	*filter_array	Array im Flash des AVRs mit den Initialisierungs-
 *							werten für die Filter des MCP2515
 * 
 * \see		MCP2515_FILTER_EXTENDED()
 * \see		MCP2515_FILTER()
 * \warning	Wird nur vom MCP2515 unterstuetzt.
 */
extern void can_static_filter(const uint8_t *filter_array);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * 
 * \~german
 * \brief	Filterdaten auslesen
 *
 * \param	number	Nummer des Filters dessen Daten man haben moechte
 * \param	*filter	Pointer in den die Filterstruktur geschrieben wird
 *
 * \return	\b 0 falls ein Fehler auftrat, \
 *			\b 1 falls der Filter korrekt gelesen werden konnte, \
 *			\b 2 falls der Filter im Moment nicht verwendet wird (nur AT90CAN), \
 *			\b 0xff falls gerade keine Aussage moeglich ist (nur AT90CAN).
 *
 * \warning	Da der SJA1000 nicht feststellen kann ob der ausgelesene Filter
 *			nun zwei 11-Bit Filter oder ein 29-Bit Filter ist werden nicht
 *			die Filter sondern die Registerinhalte direkt zurück gegeben.
 *			Der Programmierer muss dann selbst entscheiden was er mit den 
 * 			Werten macht.
 *
 * \~english
 * \warning SJA1000 doesn't return the filter and id directly but the content
 *			of the corresponding registers because it is not possible to
 *			check the type of the filter.
 */
extern uint8_t can_get_filter(uint8_t number, tCANFilter *filter);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Ueberpruefen ob neue CAN Nachrichten vorhanden sind
 *
 * \return	true falls neue Nachrichten verfuegbar sind, false ansonsten.
 */
bool can_check_message(void) {
    return true;
}

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Ueberprueft ob ein Puffer zum Versenden einer Nachricht frei ist.
 *
 * \return	true falls ein Sende-Puffer frei ist, false ansonsten.
 */
bool can_check_free_buffer(void) {
    return true;
}

extern uint8_t can_buffers_status(void);
// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Verschickt eine Nachricht über den CAN Bus
 *
 * \param	msg	Nachricht die verschickt werden soll
 * \return	FALSE falls die Nachricht nicht verschickt werden konnte, \n
 *			ansonsten der Code des Puffes in den die Nachricht gespeichert wurde
 *           Returns buffer number or 0
 */
uint8_t can_send_message(const tCAN *msg) {
    CAN_message_t m;
    //FlexCANv1 m;
    m.id = msg->id;
    m.ext = msg->flags.extended;
    m.len = msg->length;
    //memcpy(m.buf, msg->data, 8);
    for(int i=0;i<8;i++) m.buf[i] = msg->data[i];
    return fcan.write(m);
}

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Liest eine Nachricht aus den Empfangspuffern des CAN Controllers
 *
 * \param	msg	Pointer auf die Nachricht die gelesen werden soll.
 * \return	FALSE falls die Nachricht nicht ausgelesen konnte,
 *			ansonsten Filtercode welcher die Nachricht akzeptiert hat.
 *           Returns filter code or 0
 */
uint8_t can_get_message(tCAN *msg) {
    CAN_message_t m;
    //FlexCANv1 m;
    fcan.read(m);
    msg->id = m.id;
    msg->flags.extended = m.ext;
    msg->flags.rtr = 0;
    msg->length = m.len;
    //memcpy( msg->data, m.buf, 8);
    for(int i=0;i<8;i++) msg->data[i] = m.buf[i];
    return 1;
}

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 *
 * \~german
 * \brief   Liest den Inhalt der Fehler-Register
 *
 * \~english
 * \brief	Reads the Contents of the CAN Error Counter
 */
extern tCANErrorRegister can_read_error_register(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup can_interface
 *
 * \~german
 * \brief   Überprüft ob der CAN Controller im Bus-Off-Status
 *
 * \return  true wenn der Bus-Off-Status aktiv ist, false ansonsten
 *
 * \warning aktuell nur auf dem SJA1000
 */
extern bool can_check_bus_off(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 *
 * \~german
 * \brief	Setzt einen Bus-Off Status zurück und schaltet den CAN Controller
 *			wieder aktiv
 *
 * \warning	aktuell nur auf dem SJA1000
 */
extern void can_reset_bus_off(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzt den Operations-Modus
 *
 * \param	mode	Gewünschter Modus des CAN Controllers
 */
extern void can_set_mode(tCANMode mode);

extern void can_regdump(void);

//#if defined (__cplusplus)
//}
//#endif
