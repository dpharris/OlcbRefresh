// Establish location of node Name and Node Decsription in memory
#define SNII_var_data &pmem->nodeName           // location of SNII_var_data EEPROM, and address of nodeName
#define SNII_var_offset sizeof(pmem->nodeName)  // location of nodeDesc

void initTables() {

  for(unsigned i=0; i<NUM_EVENT; i++) event[i].flags |= eventOffset[i].flags;
}

