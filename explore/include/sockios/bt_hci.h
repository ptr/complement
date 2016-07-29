// -*- C++ -*-

/*
 * Copyright (c) 2016
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __SOCKIOS_BT_HCI_H
#define __SOCKIOS_BT_HCI_H

namespace bt {

namespace hci {

/* HCI Packet types */
enum {
  COMMAND_PKT = 0x01,
  ACLDATA_PKT	= 0x02,
  SCODATA_PKT	= 0x03,
  EVENT_PKT   = 0x04,
  VENDOR_PKT  = 0xff
};

class sock_filter
{
  public:
    enum {
      SO_HCI_FILTER = 2
    };

  private:
    enum {
      type_mask = 0x1f, // 1f  -> 1f >> 5 == 0
      ev_mask   = 0x3f, // 3f  -> 3f >> 5 == 1
      ogf_mask  = 0x3f, // 3f  -> 3f >> 5 == 1
      ocf_mask  = 0x7f  // 7f  -> 7f >> 5 == 3
    };

    void set_bit(int nr, void *addr)
      {	*(reinterpret_cast<uint32_t *>(addr) + (nr >> 5)) |= (1 << (nr & 0x1f)); }

    void clear_bit(int nr, void *addr)
      { *(reinterpret_cast<uint32_t *>(addr) + (nr >> 5)) &= ~(1 << (nr & 0x1f)); }

    int test_bit(int nr, void *addr)
      { return *(reinterpret_cast<uint32_t *>(addr) + (nr >> 5)) & (1 << (nr & 0x1f)); }

    
  public:

    void clear()
      { memset(this, 0, sizeof(sock_filter)); }
    
    void set_ptype(int t)
      {	set_bit((t == VENDOR_PKT) ? 0 : (t & type_mask), &_type); }
    
    void clear_ptype(int t)
      { clear_bit((t == VENDOR_PKT) ? 0 : (t & type_mask), &_type); }
    
    int test_ptype(int t)
      { return test_bit((t == VENDOR_PKT) ? 0 : (t & type_mask), &_type); }
    
    void all_ptypes()
      { _type = ~0; }
    
    void set_event(int e)
      {	set_bit((e & ev_mask), &_event); }
    
    void clear_event(int e)
      {	clear_bit((e & ev_mask), &_event); }
    
    int test_event(int e)
      {	return test_bit((e & ev_mask), &_event); }
    
    void all_events()
      {	_event[0] = _event[1] = ~0; }
    
    void opcode(int op)
      {	_opcode = op; }
    
    void opcode()
      {	_opcode = 0; }
    
    bool test_opcode(int op)
      {	return (_opcode == op); }

  private:
    uint32_t _type;
    uint32_t _event[2];
    uint16_t _opcode;
};

} // namespace hci

} // namespace bt

#endif // __SOCKIOS_BT_HCI_H
