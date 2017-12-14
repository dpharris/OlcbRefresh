/* 

OpenLCB / LCC in Brief

This is meant to be a very short summary of OpenLCB / LCC (NMRA's new Layout Command Control bus).

OpenLCB/LCC is a local area bus for model railway layouts, and used for the control of layout accessories and trains.

OpenLCB/LCC is designed to operate over many transports, one of which is CAN (Controller Area Network) which is used in many cars and trucks. OpenLCB is also designed to run trains over many traction systems, one of which is DCC. It consists of a layered set of Protocols, and these layers roughly correspond to the OSI layers: physical; link; network; transport; and application. Website is: OpenLCB

An OpenLCB/LCC installation consists of a number of 'nodes' which send messages to each other. It uses the 
  'Producer-Consumer Model' with 'events'. Some change on the layout will trigger a 'Producer' node to 
  broadcast an event-message to one or more 'Consumer' nodes – these then cause some action(s) to take place. 
  For example, a occupancy detector causes the node to which it is connected to send an event-message. 
  This message is received by another node which controls a signal, and causes that signal to display Red. 
  These messages can be produced by zero or more producers and consumed by zero to more consumers, 
  i.e. it is a many-to-many relationship.

In addition, OpenLCB/LCC has other specialized messages which are directed at specified nodes and carry 
  arbitrary data, including:
   * Datagram - these transmit up to 80 bytes to a specific destination node.
   * Stream messages - these allow continuous data to be sent to a specific destination node.
 
There are also messages systems-work, including:
   * Discovery - nodes announce themselves at start-up.
   * Self-description - nodes contain a xml description of themselves
   * Configuration - nodes can be configured by tools which use the xml to build a GUI.
   * Teaching - to establish a PC relationship, one node can teach one of its eventIDs to one or more other nodes
   * Interest-based routing - each node announces the eventIDs that it is
   * Filtering support - if a sub-network does not have interest in some messages, they do not need to be forwarded on it.

A set of OpenLCB Protocols has been adopted by the NMRA as LCC - Layout Command Control – “DCC for the rest of your layout”. See: NMRA Standards

Ongoing development higher-level protocols, such as:
   * Abstracted Traction Control, the train is a node,
   * Signalling,
   * Who and Where - identification and location services,
   * Auto-configuration - set of well-known eventIDs to connect modules at a train-meet.

And tools:
   * Graphical UI,
   * Monitoring, and
   * Operations.

Identifiers:
  * NodeIDs are 48-bit, are globally unique, and are assigned at manufacture.
      Ranges of NodeIDs are pre-assigned to individuals, groups, and companies,
      but ranges can be requested by anyone. 
  * Events are 64-bit, are globally unique, and each node is assigned 64k events. 
      Some events are 'well-known' and are used for system purposes,
      e.g. [01.00.00.00.00.00.FF.FF] is Emergency Off.

Message Formats:
  General format: [ MTI, sourceNodeID, {destinationNodeID / eventID}, {length, optional-data} ]
    Where:
      MTI is the 16-bit Message-Type-Indicator
      sourceNodeID and destinationNodeID are 48-bit NodeIDs;
      eventID is 64-bit.
      There are many MTIs, which implement:

Tools:
  The OpenLCB Group - general Github for OpenLCB software.
   * JMRI: Java Model Railroad Interface for node and event configuration, layout monitoring and operating. The Java implementation of OpenLCB lives at Github.
   * OpenLCB Test Suite: Python, for testing protocol compliance. Now lives on Github.
   * Arduino Libraries: Implementation suitable for smaller processors, such as AVR, PIC, etc. And now on Github.
   * Extensions to Arduino libraries – implements virtual Nodes as objects.
   * TCH-Technology Support - support files for TCH boards.
   * Pascal libraries: Multithreaded implementation for dsPICs.
   * OpenMRN: Multithreaded implementation with virtual nodes and suitable for PCs and larger microprocessors, 
       such as ARM and PIC32.
   * Model Railroad System: support for a network of Bruce Chubb CMR/I, Lenz’s XPressNet DCC nodes, Azatrax 
       USB-connected Model Railroad Detectors, a CTI Acela network, and OpenLCB.

Behind the scenes:

  General message formats: (eg on Ethernet)
    * Event-messages: [ MTI, Source-node ID, Event# ]
    * Global-messages: [ MTI, Source-node ID, Length, Data ]
    * Directed-messages: [ MTI, Source-node ID, Destination-node ID, Length, Data]
    Where:
      MTI is the 16-bit Message Type Indicator;
      Source-node and destination-node IDs are 48-bit.

  In the short-term most development has focused on CAN, and most current nodes use CAN. The general formats have been adapted to use CAN's advantages and to mitigate its space limitations. These are detailed below.
  On CAN:
    * CAN is operated at 125kbps.
    * CAN is bidirectional and has built-in hardware error correction and message re-send.
    * OpenLCB CAN frames have a 29-bit header and a 8-byte data-part:
    * Messages are carried in the entire CAN frame, and long messages are fragmented into multiple frames.
    * CAN requires the header to be unique - this is ensured by including srcAlias, the Alias of the sending Node.
    * A 12-bit CAN-MTI is mapped from the general 16-bit MTI.
    * A 12-bit srcAlias is mapped one-to-one to a sourceNodeID.
    * A 12-bit dstAlias is mapped one-to-one to a destinationNodeID.
    * Aliases are obtained at start-up by a Node randomly choosing a candidate alias; ensuring it is not in use; 
        and claiming it.
    * Global messages are sent as: [CAN-MTI.srcAlias] optional-data
        where […] is the header, the rest is the data-part. 
        Eg: Events are sent as: [CAN-MTI, srcAlias] event#.
    * Most directed-messages are sent as: [CAN-MTI.srcAlias] ff.dstnAlias, optional-data
        where […] is the header, the rest is the data-part, and ff is fragmentation control-bits.
    * Datagrams are sent as : [FC.dstAlias.srcAlias] type, data
        where the FC bit-field determines both that this is a a datagram-frame and also indicates 
        fragmentation control, and type indicates the kind of datagram.
    * Streams are sent as: [S.dstAlias.srcAlias] data
        where the S bit-field indicates a stream-frame. Streams are limited to one between any 
        source-node and destination-node.

*/
