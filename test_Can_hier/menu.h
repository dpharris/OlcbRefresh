String m1[] = { "Connection:", "< speed >", "Loco:" };
String m2[] = { "0000","0000", "0000", "0000", "Loco", "F0:", "F1:", "F2:", "F3:", "F4:", "F5:", 
                "F6:", "F7:", "F8:", "F9:", "F10:", "F11:", "F12:", "F13:", "F14:", "F15:", "Setup" };
bool iclick = false;
bool iup = false;
bool idown = false;
bool click(){ if(iclick) { iclick=false; return true; } return false; }
bool up(){ if(iup) { iup=false; return true; } return false; }
bool down(){ if(idown) { idown=false; return true; } return false; }

int locoNum = 0;
int locoSpeed = 0;

bool menu3() {
  static int cursor = 0;
  int num = 0;
  if(num<1000) P(0); if(num<100) P(0); if(num<10) P(0); P(num);
  if(cursor==3) P("\n   ^"); 
  else if(cursor==2) P("\n  ^"); 
  else if(cursor==1) P("\n ^"); 
  else P("\n^");
  if(click()) cursor++;
  if(up()) num += (int)pow(10.,4.-cursor);
  if(down()) num -= (int)pow(10.,4.-cursor);
  if(num>9999) num=9999;
  if(num<0) num=0;
  if(cursor==4) {
    locoNum = num;
    return true;
  }
  return false;
}

#define maxitems 5
bool menu2() {
  static uint8_t icursor = 4;
  static uint8_t sub = 0;
  int sM = 22;
  // sub-menus
  if(sub==4) {
    if(menu3()) return true;
    return false;
  }
  // display
  int istart = icursor-(maxitems/2);
  int iend = istart+maxitems;
  for(int i=istart; i<iend; i++) {
    int ip = i%sM;
    if(ip>sM) ip=sM;
    if(ip<0) ip=0;
    P( (ip==icursor?"\n>":"\n ") );
    P( m2[ip] );
  }
  // actions
  if(click()) {
    switch (icursor) {
      case 0: case 1: case 2: case 3: 
        locoNum = m2[icursor].toInt();
        return true;
      case 4:
        sub=4;
        break;
    }
  }
  if(up()) icursor--;
  if(down()) icursor++;
  return false;
}

String conn("None");

bool menu1(){
  static uint8_t sub = 0;
  if(sub==1) {
    if(menu2()) return true;
    return false;
  } else {
    P("\nConnection: "); P(conn); PL;
    (locoSpeed<0)?P("< "):P("  "); P(locoSpeed); if(locoSpeed>0) P(" >");
    P("\nLoco: "); P(locoNum);
    if(click()) sub=1;
    if(up()) locoSpeed++;
    if(down()) locoSpeed--;
  }
  return true;
}

void menu_process() {
  char c;
  if((c = Serial.read())>0) {
    if(c==' ') iclick=true;
    else if(c==',') iup=true;
    else if(c=='.') idown=true;
    else return;
    P("\n");
    menu1();
  }
}

