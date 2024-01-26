/**
*********************************************************************************************************
* Copyright (C) 2023-2026 by xiongqulin - All Rights Reserved.                                              *
*                                                                                                       *
* This file is part of the project.                                                                     *
*                                                                                                       *
* This file can not be copied and/or distributed without the express permission of the project owner.   *
*********************************************************************************************************/

/**
*********************************************************************************************************
* @file   : led.hpp
* @author : xiongqulin
* @date   : 19 Jan 2024
* @brief  :
*
*********************************************************************************************************
*/

#pragma once

#include <cstdint>
#include "infrastructure/component/common/os.hpp"
#include "infrastructure/platform/hal/uart.hpp"

class Dr16 {
public:
    struct RcMessage {
        int16_t ch0;
        int16_t ch1;
        int16_t ch2;
        int16_t ch3;
        uint8_t s_left;
        uint8_t s_right;
    };

    struct MouseMessage {
        int16_t x;
        int16_t y;
        int16_t z;
        uint8_t left;
        uint8_t right;
    };

    struct KeyboardMessage {
        uint8_t w : 1;
        uint8_t s : 1;
        uint8_t a : 1;
        uint8_t d : 1;
        uint8_t shitf : 1;
        uint8_t ctrl : 1;
        uint8_t q : 1;
        uint8_t e : 1;

        uint8_t r : 1;
        uint8_t f : 1;
        uint8_t g : 1;
        uint8_t z : 1;
        uint8_t x : 1;
        uint8_t c : 1;
        uint8_t v : 1;
        uint8_t b : 1;
    };

    struct Message {
        RcMessage rc;
        MouseMessage mouse;
        KeyboardMessage key;
    };

private:
    Uart &_uart;
    os::Semaphore _sem;
    os::Mutex _mutex;
    Message _message;
    uint8_t _rece_mem[sizeof(Message)];
    uint16_t _mem_index = 0;

public:
    Dr16(Uart &uart)
        : _uart(uart)
    {
        _uart.attach_data_callback(receive_data, this);
        _uart.attach_idle_callback(receive_idle, this);
    }

    void receive_start()
    {
        _uart.read_start(_rece_mem, sizeof(_rece_mem));
    }

    void decode()
    {
        _sem.get();
        _mutex.lock();
        memcpy(reinterpret_cast<uint8_t *>(&_message), _rece_mem, sizeof(Message));
        _mutex.unlock();
    }

    void message_lock()
    {
        _mutex.lock();
    }

    void message_unlock()
    {
        _mutex.unlock();
    }

    const RcMessage get_rc_message()
    {
        static RcMessage rc_msg;
        _mutex.lock();
        memcpy(reinterpret_cast<uint8_t *>(&rc_msg), reinterpret_cast<uint8_t *>(&_message.rc), sizeof(RcMessage));
        _mutex.unlock();
        return rc_msg;
    }

    const MouseMessage get_mouse_message()
    {
        static MouseMessage mouse_msg;
        _mutex.lock();
        memcpy(reinterpret_cast<uint8_t *>(&mouse_msg), reinterpret_cast<uint8_t *>(&_message.mouse), sizeof(MouseMessage));
        _mutex.unlock();
        return mouse_msg;
    }

    const KeyboardMessage get_keyboard_message()
    {
        static KeyboardMessage key_msg;
        _mutex.lock();
        memcpy(reinterpret_cast<uint8_t *>(&key_msg), reinterpret_cast<uint8_t *>(&_message.key), sizeof(KeyboardMessage));
        _mutex.unlock();
        return key_msg;
    }

public:
    static void receive_data(void *context, uint8_t *buf, uint16_t len)
    {
        Dr16 *me = static_cast<Dr16 *>(context);
        memcpy(&me->_rece_mem[me->_mem_index], buf, len);
        me->_mem_index += len;
    }

    static void receive_idle(void *context)
    {
        Dr16 *me = static_cast<Dr16 *>(context);

        me->_mem_index = 0;
        me->_sem.put();
    }
};