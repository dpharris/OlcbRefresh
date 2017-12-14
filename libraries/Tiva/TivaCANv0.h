/*
 * Copyright (C) 2016 by Stuart W Baker
 * CAN library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _CAN_H_INCLUDED
#define _CAN_H_INCLUDED

#include <Arduino.h>
#include <Energia.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

/// CAN frame structure to be used with @ref CAN::read() and CAN::write().
typedef struct CAN_message_t
{
    uint32_t id : 29; ///< CAN identifier
    uint32_t rtr : 1; ///< this is a remote frame
    uint32_t err : 1; ///< this is an error frame
    uint32_t eff : 1; ///< identifier is extended
    uint32_t timeout; ///< timeout in milliseconds, zero will disable waiting
    uint8_t dlc; ///< data length code (0-8)
    uint8_t buf[8]; ///< CAN payload data
} CAN_message_t;

/// CAN module class.
class CANClass
{
private:
    struct RingBuffer
    {
        static const uint8_t SIZE = 8;
        CAN_message_t Buf[SIZE];
        uint8_t Count;
        uint8_t RdIndex;
        uint8_t WrIndex;
    };
    
    RingBuffer TxBuffer;
    RingBuffer RxBuffer;
    
    uint32_t Base;
    uint32_t Interrupt;
    uint32_t Baud;
    bool TxPending;
    uint8_t Data[8];
    
public:
    /// Constructor.
    /// @param module CAN module instance on device (0 or 1)
    /// @param baud CAN module baud rate
    CANClass(uint8_t module, uint32_t baud = 125000);
    
    /// Initiailize and enable CAN module.
    void begin();
    
    /// Deinit and disable CAN module
    void end();
    
    /// @returns number of available CAN messages in the receive queue
    int available();
    
    ////// Returns true if transmit buffer is free
    bool tx_idle();
    
    /// Write one or more CAN messages to the transmit queue.
    /// @param msg reference to one or more CAN messages
    /// @param count number of CAN messages to write
    /// @return number of CAN messages written
    int write(const CAN_message_t msg[], size_t count = 1);
    
    /// Read one or more CAN messages from the receive queue.
    /// @param msg reference to one or more CAN messages
    /// @param count number of CAN messages to read
    /// @return number of CAN messages read
    int read(CAN_message_t msg[], size_t count = 1);
    
    void CANIntHandler();
};

#endif
