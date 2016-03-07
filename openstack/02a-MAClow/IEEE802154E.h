#ifndef __IEEE802154E_H
#define __IEEE802154E_H

/**
\addtogroup MAClow
\{
\addtogroup IEEE802154E
\{
*/

#include "opendefs.h"
#include "board.h"
#include "schedule.h"

//=========================== debug define ====================================

//=========================== static ==========================================
static const uint8_t chTemplate_default[] = {
   5,6,12,7,15,4,14,11,8,0,1,2,13,3,9,10
};

static const uint8_t chTemplate_eb[] = {
   //0,4,9,15                            // channels to send EBs on (-11, i.e. 0=channel 11) (0,4,9,15)==(11,15,20,26)
  14
};
//=========================== define ==========================================

#define EB_NUMCHANS               1  // number of channels EBs are sent on
#define EB_SLOWHOPPING_PERIOD     500  // how often a node changes the channel it listens on for EBs, in slots (500=7500ms)

#define DEFAULT_BLACKLIST         0xFF00

#define SHORTTYPE_UNDEFINED       0xff
#define SHORTTYPE_BEACON          0xb0
#define SHORTTYPE_DATA            0xd0
#define SHORTTYPE_ACK             0xa0  

#define LONGTYPE_UNDEFINED        0xffff
#define LONGTYPE_BEACON           0xb0b0
#define LONGTYPE_DATA             0xd0d0
#define LONGTYPE_ACK              0xa0a0  

#define SYNCHRONIZING_CHANNEL       25 // channel the mote listens on to synchronize
#define TXRETRIES                    3 // number of MAC retries before declaring failed
#define TX_POWER                    31 // 1=-25dBm, 31=0dBm (max value)
#define RESYNCHRONIZATIONGUARD      60 // in 32kHz ticks. min distance to the end of the slot to successfully synchronize
#define US_PER_TICK                 30 // number of us per 32kHz clock tick
#define EBPERIOD                  1000 // in ms, EB sending period
#define MAXKAPERIOD                200 // in slots: @15ms per slot -> ~30 seconds. Max value used by adaptive synchronization.
#define DESYNCTIMEOUT              800 // in slots: 2169@15ms per slot -> ~10 seconds
#define LIMITLARGETIMECORRECTION     5 // threshold number of ticks to declare a timeCorrection "large"
#define LENGTH_IEEE154_MAX         128 // max length of a valid radio packet  
#define DUTY_CYCLE_WINDOW_LIMIT    (0xFFFFFFFF>>1) // limit of the dutycycle window

#define EB_PERIOD_TIMER   2000 // every 2 seconds increase the EB period by a certain amount
#define EB_PERIOD_AMOUNT     2 // increase by 1 (multiples of 100 ms)
#define EB_PERIOD_MAX       20 // max EB interval will be 10 (multiples of 100 ms)

//15.4e information elements related
#define IEEE802154E_PAYLOAD_DESC_LEN_SHIFT                 0x04
#define IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME             (1<<11)
#define IEEE802154E_PAYLOAD_DESC_TYPE_MLME                 (1<<15)
#define IEEE802154E_DESC_TYPE_LONG                         (1<<15)
#define IEEE802154E_DESC_TYPE_SHORT                        (0<<15)

#define IEEE802154E_DESC_TYPE_HEADER_IE                    0x0000
#define IEEE802154E_DESC_TYPE_PAYLOAD_IE                   0x8000
//len field on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_LEN_HEADER_IE_MASK                0x007F
#define IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK               0x07FF

//groupID/elementID field on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK          0x7F80
#define IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK           0x7800

#define IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT         7
#define IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT          11

//type field on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_TYPE_IE_MASK                      0x8000

#define IEEE802154E_DESC_TYPE_IE_SHIFT                     15

//MLME Sub IE LONG page 83
#define IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK             0x07FF
#define IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK           0x7800

#define IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT          11

//MLME Sub IE SHORT page 82
#define IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK            0x00FF
#define IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK          0x7F00

#define IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT         8

#define IEEE802154E_MLME_SYNC_IE_SUBID                     0x1A
#define IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT               8
#define IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID           0x1B
#define IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID_SHIFT     8
#define IEEE802154E_MLME_TIMESLOT_IE_SUBID                 0x1c
#define IEEE802154E_MLME_TIMESLOT_IE_SUBID_SHIFT           8
#define IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID           0x09
#define IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID_SHIFT     11

#define IEEE802154E_MLME_IE_GROUPID                        0x01
#define IEEE802154E_ACK_NACK_TIMECORRECTION_ELEMENTID      0x1E

/**
When a packet is received, it is written inside the OpenQueueEntry_t->packet
buffer, starting at the byte defined below. When a packet is relayed, it
traverses the stack in which the MAC and IPHC headers are parsed and stripped
off, then put on again. During that process, the IPv6 hop limit field is
decremented. Depending on the new value of the hop limit, the IPHC header
compression algorithm might not be able to compress it, and hence has to carry
it inline, adding a byte to the header. To avoid having to move bytes around
inside OpenQueueEntry_t->packet buffer, we start writing the received packet a
bit after the start of the packet.
*/
#define FIRST_FRAME_BYTE             1

// the different states of the IEEE802.15.4e state machine
typedef enum {
   S_SLEEP                   = 0x00,   // ready for next slot
   // synchronizing
   S_SYNCLISTEN              = 0x01,   // listened for packet to synchronize to network
   S_SYNCRX                  = 0x02,   // receiving packet to synchronize to network
   S_SYNCPROC                = 0x03,   // processing packet just received
   // TX
   S_TXDATAOFFSET            = 0x04,   // waiting to prepare for Tx data
   S_TXDATAPREPARE           = 0x05,   // preparing for Tx data
   S_TXDATAREADY             = 0x06,   // ready to Tx data, waiting for 'go'
   S_TXDATADELAY             = 0x07,   // 'go' signal given, waiting for SFD Tx data
   S_TXDATA                  = 0x08,   // Tx data SFD received, sending bytes
   S_RXACKOFFSET             = 0x09,   // Tx data done, waiting to prepare for Rx ACK
   S_RXACKPREPARE            = 0x0a,   // preparing for Rx ACK
   S_RXACKREADY              = 0x0b,   // ready to Rx ACK, waiting for 'go'
   S_RXACKLISTEN             = 0x0c,   // idle listening for ACK
   S_RXACK                   = 0x0d,   // Rx ACK SFD received, receiving bytes
   S_TXPROC                  = 0x0e,   // processing sent data
   // RX
   S_RXDATAOFFSET            = 0x0f,   // waiting to prepare for Rx data
   S_RXDATAPREPARE           = 0x10,   // preparing for Rx data
   S_RXDATAREADY             = 0x11,   // ready to Rx data, waiting for 'go'
   S_RXDATALISTEN            = 0x12,   // idle listening for data
   S_RXDATA                  = 0x13,   // data SFD received, receiving more bytes
   S_TXACKOFFSET             = 0x14,   // waiting to prepare for Tx ACK
   S_TXACKPREPARE            = 0x15,   // preparing for Tx ACK
   S_TXACKREADY              = 0x16,   // Tx ACK ready, waiting for 'go'
   S_TXACKDELAY              = 0x17,   // 'go' signal given, waiting for SFD Tx ACK
   S_TXACK                   = 0x18,   // Tx ACK SFD received, sending bytes
   S_RXPROC                  = 0x19,   // processing received data
} ieee154e_state_t;

#define  TIMESLOT_TEMPLATE_ID         0x00
#define  CHANNELHOPPING_TEMPLATE_ID   0x00

// Atomic durations
// expressed in 32kHz ticks:
//    - ticks = duration_in_seconds * 32768
//    - duration_in_seconds = ticks / 32768
enum ieee154e_atomicdurations_enum {
   // time-slot related
   TsTxOffset                =  131,                  //  4000us
   TsLongGT                  =   43,                  //  1300us
   TsTxAckDelay              =  151,                  //  4606us
   TsShortGT                 =  16,                   //     500us
   TsSlotDuration            =  PORT_TsSlotDuration,
   // execution speed related
   maxTxDataPrepare          =  PORT_maxTxDataPrepare,
   maxRxAckPrepare           =  PORT_maxRxAckPrepare,
   maxRxDataPrepare          =  PORT_maxRxDataPrepare,
   maxTxAckPrepare           =  PORT_maxTxAckPrepare,
   // radio speed related
   delayTx                   =  PORT_delayTx,         // between GO signal and SFD
   delayRx                   =  PORT_delayRx,         // between GO signal and start listening
   // radio watchdog
   wdRadioTx                 =   33,                  //  1000us (needs to be >delayTx)
   wdDataDuration            =  164,                  //  5000us (measured 4280us with max payload)
   wdAckDuration             =  98,                   //  3000us (measured 1000us)
};

//shift of bytes in the linkOption bitmap: draft-ietf-6tisch-minimal-10.txt: page 6
enum ieee154e_linkOption_enum {
   FLAG_TX_S                 = 0,
   FLAG_RX_S                 = 1,
   FLAG_SHARED_S             = 2,
   FLAG_TIMEKEEPING_S        = 3,   
};

enum {
   ENERGY_RSSI               = 0,
   ENERGY_NOISE              = 1,
};

// FSM timer durations (combinations of atomic durations)
// TX
#define DURATION_tt1 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx-maxTxDataPrepare
#define DURATION_tt2 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx
#define DURATION_tt3 ieee154e_vars.lastCapturedTime+TsTxOffset-delayTx+wdRadioTx
#define DURATION_tt4 ieee154e_vars.lastCapturedTime+wdDataDuration
#define DURATION_tt5 ieee154e_vars.lastCapturedTime+TsTxAckDelay-TsShortGT-delayRx-maxRxAckPrepare
#define DURATION_tt6 ieee154e_vars.lastCapturedTime+TsTxAckDelay-TsShortGT-delayRx
#define DURATION_tt7 ieee154e_vars.lastCapturedTime+TsTxAckDelay+TsShortGT
#define DURATION_tt8 ieee154e_vars.lastCapturedTime+wdAckDuration
// RX
#define DURATION_rt1 ieee154e_vars.lastCapturedTime+TsTxOffset-TsLongGT-delayRx-maxRxDataPrepare
#define DURATION_rt2 ieee154e_vars.lastCapturedTime+TsTxOffset-TsLongGT-delayRx
#define DURATION_rt3 ieee154e_vars.lastCapturedTime+TsTxOffset+TsLongGT
#define DURATION_rt4 ieee154e_vars.lastCapturedTime+wdDataDuration
#define DURATION_rt5 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx-maxTxAckPrepare
#define DURATION_rt6 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx
#define DURATION_rt7 ieee154e_vars.lastCapturedTime+TsTxAckDelay-delayTx+wdRadioTx
#define DURATION_rt8 ieee154e_vars.lastCapturedTime+wdAckDuration

//=========================== typedef =========================================

// IEEE802.15.4E acknowledgement (ACK)
typedef struct {
   PORT_SIGNED_INT_WIDTH timeCorrection;
} IEEE802154E_ACK_ht;

// includes payload header IE short + MLME short Header + Sync IE
#define EB_PAYLOAD_LENGTH sizeof(payload_IE_ht) + \
                           sizeof(mlme_IE_ht)     + \
                           sizeof(sync_IE_ht)

//=========================== module variables ================================

typedef struct {
   // misc
   asn_t                     asn;                     // current absolute slot number
   slotOffset_t              slotOffset;              // current slot offset
   uint8_t                   syncnum;                 // current synchronization number
   slotOffset_t              nextActiveSlotOffset;    // next active slot offset
   PORT_RADIOTIMER_WIDTH     deSyncTimeout;           // how many slots left before looses sync
   bool                      isSync;                  // TRUE iff mote is synchronized to network
   OpenQueueEntry_t          localCopyForTransmission;// copy of the frame used for current TX
   // as shown on the chronogram
   ieee154e_state_t          state;                   // state of the FSM
   OpenQueueEntry_t*         dataToSend;              // pointer to the data to send
   OpenQueueEntry_t*         dataReceived;            // pointer to the data received
   OpenQueueEntry_t*         ackToSend;               // pointer to the ack to send
   OpenQueueEntry_t*         ackReceived;             // pointer to the ack received
   PORT_RADIOTIMER_WIDTH     lastCapturedTime;        // last captured time
   PORT_RADIOTIMER_WIDTH     syncCapturedTime;        // captured time used to sync
   // channel hopping
   uint8_t                   freq;                    // frequency of the current slot
   uint8_t                   ebAsnOffset;               // offset inside the frame for eb
   uint8_t                   dataAsnOffset;           // offset inside the frame for data
   uint8_t                   singleChannel;           // the single channel used for transmission
   bool                      singleChannelChanged;    // detect id singleChannelChanged
   uint8_t                   chTemplate[16];          // storing the template of hopping sequence
   uint8_t                   chTemplateEB[EB_NUMCHANS];    // hopping sequence for EB
   // template ID
   uint8_t                   tsTemplateId;            // timeslot template id
   uint8_t                   chTemplateId;            // channel hopping tempalte id
   
   PORT_RADIOTIMER_WIDTH     radioOnInit;             // when within the slot the radio turns on
   PORT_RADIOTIMER_WIDTH     radioOnTics;             // how many tics within the slot the radio is on
   bool                      radioOnThisSlot;         // to control if the radio has been turned on in a slot.
   
   //control
   bool                      isAckEnabled;            // whether reply for ack, used for synchronization test
   bool                      isSecurityEnabled;       // whether security is applied
   // time correction
   int16_t                   timeCorrection;          // store the timeCorrection, prepend and retrieve it inside of frame header
   uint16_t                  syncSlotLength;
   // flooding counter
   uint16_t                  floodingCounter;
   // flooding state
   bool                      floodingState;
   // next channel to send beacon
   uint8_t                   nextChannelEB;
   uint8_t                   jumpCounter;
   
   uint16_t                  joinChannelChangingCounter;
} ieee154e_vars_t;

BEGIN_PACK
typedef struct {
   uint8_t                   numSyncPkt;              // how many times synchronized on a non-ACK packet
   uint8_t                   numSyncAck;              // how many times synchronized on an ACK
   int16_t                   minCorrection;           // minimum time correction
   int16_t                   maxCorrection;           // maximum time correction
   uint8_t                   numDeSync;               // number of times a desync happened
   uint32_t                  numTicsOn;               // mac dutyCycle
   uint32_t                  numTicsTotal;            // total tics for which the dutycycle is computed
} ieee154e_stats_t;
END_PACK

typedef struct {
   PORT_RADIOTIMER_WIDTH     num_newSlot;
   PORT_RADIOTIMER_WIDTH     num_timer;
   PORT_RADIOTIMER_WIDTH     num_startOfFrame;
   PORT_RADIOTIMER_WIDTH     num_endOfFrame;
} ieee154e_dbg_t;

//=========================== prototypes ======================================

// admin
void               ieee154e_init(void);
// public
PORT_RADIOTIMER_WIDTH   ieee154e_asnDiff(asn_t* someASN);
bool               ieee154e_isSynch(void);
void               ieee154e_getAsn(uint8_t* array);
void               ieee154e_getAsnStruct(asn_t* toAsn);
void               ieee154e_setIsAckEnabled(bool isEnabled);
void               ieee154e_setSingleChannel(uint8_t channel);

uint16_t           ieee154e_getTimeCorrection(void);
uint8_t            ieee154e_getEnergy(uint8_t type);

// events
void               ieee154e_startOfFrame(PORT_RADIOTIMER_WIDTH capturedTime);
void               ieee154e_endOfFrame(PORT_RADIOTIMER_WIDTH capturedTime);
// misc
bool               debugPrint_asn(void);
bool               debugPrint_isSync(void);
bool               debugPrint_macStats(void);
/**
\}
\}
*/

#endif
