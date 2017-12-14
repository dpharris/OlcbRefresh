/*
 * OpenLCB is a set of layered protocols, and
 * these layers are reflected in a set of *.h and *.cpp files.  
 *
 * Link: The lowest level, LinkControl is reponsible for maintaining
 *    a connection with the OpenLCB network.  
 *    This includes: 
 *      - obtaining an alias for this node, and updating it, if necessary
 *      - reporting that the node is up
 *      - reporting the events that this node produces and consumes
 *      - reponding to requests: node, events, CDI
 *
 *  The communiction between the system-code and the app is by way of an index of teh eventid.  
 *      The user app is responsible for the following:
 *      - The app needs to construct the list of event-offsets at start-up.
 *      - The app needs to generate the index of any producer-events that are triggered.
 *      _ The app needs to respond to the index of a consumed-event with the appropriate action.
 *
 *
 *
 * Memory use:
 *   eeprom holds non-volatile variables including NID, events and other variables according to MemStruct
 *   This is considered to be the 'true' data store, and all changes to MemStruct data are made to the eeprom.
 *   
 * 3 RAM schemes:  there are three models for RAM usage, each has advantages and disdvantages:
 *   Large Model: entire eeprom is mirrored in RAM  
 *     adv: easy access. fast access using compile-time syntactic address construction
 *     disadv: uses the most memory, must be kept in sync with the eeprom
 *   Medium Model: only eventIDs are copied to RAM
 *     adv: fast event processing, smaller footprint
 *     disadv: need to sync the eventIDs with eeprom, slow access to other variables
 *   Small Model: almost none of eeprom is mirrored
 *     adv: smallest footprint, almost no sync required
 *     disadv: slower processing speed
 *     
 * All models use: 
 *  1. MemStruct the struct of the eeprom, and also the ram in the large memory model
 *  2. eventsIndex[] an array of (eventid-hash, index)
 *  3. events[] an array of eventid offsets in Memstruct, and flags for marking/actions.
 *  4. In the small and large models, the relations between this items are:
 *          eventsIndex[].index--->events[].offset---->mem/EEPROM
 *  4. In the medium-model, the nodes' eventIDs are kept in ram in eventids[], 
 *     in this case, eventOffsets[] is not used, rather the events[].index indexes into 
 *     the eventids[] entry directly, so the relation is: 
 *          events[].index ------>eventids[]
 *      OR inclide .eventid in events[].
 *     
 * All schemes use a pointer to MemStruct to allow syntactic access to variables:
 *          MemStruct *pmem = 0;     // zero-based pointer
 *  1. Large-model:
 *          int r = mem[&pmem=>channel[i].rate];          // ram access
 *  2. All models use a similar syntax for eeprom access: 
 *          int r = EEPROM.read(&pmem->channel[i].rate);  // retrieve channels i's rate
 *          EEPROM.write(&pmem->channel[i].rate, 12);     // set channel i's rate to 12
 *   
 * The class Event incorporates the hash of each eventID, its sequential index, and type/action flags
 *   Events events[N];
 *   To speed searching, events[] can be sorted on the hash calues, 
 *   the index field then allows reference back to the eventIDs, either via an offset into MemStruct, or 
 *   an array of eventids[] in the medium-model.    
 * An associated array of eventOffsets[] into Memstruct is also constructed.  
 *   This is used by the Olcb code to access eventIDs in MemStuct in eeprom or ram.  
 *   - in the large model, the index points into the eventOffset array, and its entry points into MemStruct mem;
 *   - in the medium model, the index points into the eventid array;
 *   - in the small model, the index points into the eventOffset array, and its entry points into eeprom.   
 *   
 * These memory models will be instrumented and some may be depreciated.  
 * 
 * One is chosen at compile time by a #define
 *  #define MEM_SMALL 0
 *  #define MEM_MEDIUM 1
 *  #define MEM_LARGE 2
 *  #define MEM_MODEL MEM_SMALL // default to small
 *   
 *   
 *   BEHIND THE SCENES:
 *   OpenLCB is quite complicated, and a lot of the routine, management activities are performed by the 
 *     OpenLCB libraries automatically.  These include: receipt, notification and replies to query 
 *     requests fro other nodes.  
 *   EventIDs that are received are passed to the application code though a callback routine, 
 *     which allows the application to process them.  This routine is passed teh sequential index of 
 *     the received eventID, and it must decode this based on the MemStruct of the node.   
 *   
 *   
 * Large:
 *   - mirror eeprom to mem
 *   - init Events[] with index
 *   - user needs to init eventsOffset[] with offsets to node's eventIDs from mem
 *   - access: mem.channel[i].rate;
 *   - sync
 * Medium:
 *   - init eventIDs[] from eeprom
 *   - init Events[] with index
 *   - user needs to init eventsOffset[] with offsets to node's eventIDs from eeprom
 *   - access: eventids[i], EEPROM.read(&pmem->channels[i].rate);
 * Small:
 *   - init Events[] with index
 *   - user needs to init eventsOffset[] with offsets to node's eventIDs from eeprom
 *   - access: EEPROM.read(&pmem->channels[i].event);
 * 
 * ideas:
 *  -- this allows olcb code to fill arrays
 *   int userEventOffset(int i) {
 *     if(i<16) return &pmem->inputs[i/2].event[i%2);
 *     else if ((i-=16)<16) return &pmem->inputs[i/2].event[i%2);
 *  -- maybe set of:
 *   EventID getEID(int offset) {
 *     EventID e;
 *     for(int i=0;i<8;i++) e.val[i] = EEPROM.read(offset);
 *     return e;
 *   }
 * -- defines:
 *  #if MEM_MODEL == SMALL
 *   #define read(o) EEPROM.read(o)
 *   #define write(o,v) EEPROM.write(o,v)
 *   int compare(void* a, void* b) {
 *     Event* ea = (Event*)a;
 *     Event* eb = (Event*)a;
 *     for(uint8_t i=0; i<8; i++)
 *       if(EEPROM.read(ea.ea+7-i) > EEPROM.read(eb.ea+7-i)) return -1;
 *       if(EEPROM.read(ea.ea+7-i) < EEPROM.read(eb.ea+7-i)) return 1;
 *     }
 *     return 0;
 *   }
 *  #endif
 *  #if MEM_MODEL == MEDIUM
 *   #define read(o) ((uint8_t*)mem)[o]
 *   #define write(o,v) ((uint8_t*)mem)[o] = v
 *   int compare(void* a, void* b) {
 *     Event* ea = (Event*)a;
 *     Event* eb = (Event*)a;
 *     uint8_t* m = (uint8_t*)eventids;   // eventids array in ram
 *     for(uint8_t i=0; i<8; i++)
 *       if(m[ea.index+7-i] > m[eb.index+7-i)] return -1;
 *       if(m[ea.index+7-i] < m[eb.index+7-i)] return 1;
 *     }
 *     return 0;
 *   }
 *  #endif
 *  #if MEM_MODEL == LARGE
 *   #define read(o) ((uint8_t*)mem)[o]
 *   #define write(o,v) ((uint8_t*)mem)[o] = v
 *   int compare(void* a, void* b) {
 *     Event* ea = (Event*)a;
 *     Event* eb = (Event*)a;
 *     uint8_t* m = (uint8_t*)mem;        // eeprom-mirror in ram
 *     for(uint8_t i=0; i<8; i++)
 *       if(m[ea.index+7-i] > m[eb.index+7-i)] return -1;
 *       if(m[ea.index+7-i] < m[eb.index+7-i)] return 1;
 *     }
 *     return 0;
 *   }
 *  #endif
 *  
 *  // small
 *  int Event::findIndexInArray(Event* events, int len, int start) {
 *    for (int e=start; e<len; e++) {
 *      int a = events[e].ea;
 *      for (int i=0;i<8;i++) if(EEPROM.read(ea+7-i)!=EEPROM.read(a+7-i)) continue;
 *      return i;
 *    }
 *    return -1;
 *  }
 *  // medium
 *  int Event::findIndexInArray(Event* events, int len, int start) {
 *    for (int e=start; e<len; e++) {
 *      int a = events[e].ea;
 *      for (int i=0;i<8;i++) if(eventids[ea+7-i]!=eventids[a+7-i]) continue;
 *      return i;
 *    }
 *    return -1;
 *  }
 *  // large
 *  int Event::findIndexInArray(Event* events, int len, int start) {
 *    uint8_t* m = (uint8_t*) mem;
 *    for (int e=start; e<len; e++) {
 *      int a = events[e].ea;
 *      for (int i=0;i<8;i++) if(m[ea+7-i]!=m[a+7-i]) continue;
 *      return i;
 *    }
 *    return -1;
 *  }
 *  
 *  2017.11.14
 *  void NodeMemory::setup()
 *    call initEventOffets() -- which calls userInitEventOffsets  (medium model eventOffests is 0...n)
 *    if nid ok, then load eeprom if necessary
 *       -- Large: mirror eeprom to mem
 *       -- Medium: copy eventIDs to eventids[]
 *       -- Small: nil
 *       -- all copy NID to nid, nextEID to nextEID;
 *    else if not all ok, then reset nextEID, doSoftReset
 *    
 *     
 *  So:   
 *  uint8_t ptrToEventID(Event* pe)  returns ptr to eventid
 *  EventID getEventID(Event* pe) {  // returns eventID
 *    e = EventID();
 *    if large:  forall e.val[i] = mem[eventOffsets[pe.index]];         // from ram-mirror
 *    if medium: forall e.val[i] = eventids[pe.index];                  // from eventids
 *    if small:  forall e.val[i] = EEPROM.read(eventOffsets[pe.index]); // from EEPROM
 *    return e;
 *  }
 *  
 *  is it better to have separate struct for
 *   typedef struct {
 *      uint16_t magic[2];
 *      uint16_t count;
 *      uint8_t  nid[8];
 *    } NodeBase;
 *   typedef struct {
 *      char nodename[20];
 *      char nodeDesc[24];
 *      struct {
 *        char desc[16];
 *        uint8_t onEvent[8];
 *        uint8_t offEvent[8];
 *      } channel[8];
 *    } MemStruct;
 *  
 *    
 */
