
#define P(x) Serial.print(x)
#define PL Serial.println()
#define submenu(m) if(m()) {sub=-1; return 1;} else return 0;

bool up = false;
bool down = false;
bool clk = false;
String loco;
int speed = 0;
bool f[12] = {false};
const int nrows = 3;
  
String selection;

void show(int c, String* s, int n) {
  P("\nshow(");P(c);P(",");P(n);P(")");
  int p = (c-nrows/2)%(nrows);
  P("\np=");P(p);
  int l = p+nrows;
  P("\nl=");P(l);
  for(int i=0;i<nrows;i++) {
    if(p<0) p=n;
    if(p>n) p=0;
    PL; 
    P( (p==c ? ">" : " ") );
    P(s[p]);  
    p++;
  }
}

uint8_t menuLoco() {
  static int n = 0;
  static int ln;
  if(n==0) ln=0;
  P("\nChoose Loco:\n");
  P(ln);
  if(up) ln++;
  if(down) ln--;
  if(clk) {
    if(++n>3) {
      loco = String(ln);
      n=0;
      return 1;
    }
    ln*=10;
  }
  return 0;
}

uint8_t menuF() {
  static int sub=-1;
  static int ptr=3;
  const int n=10;
  String items[] = { "----","----","---","Run","Loco>","F0","F1","F2","F3","F4" };
  if(sub==5) submenu(menuLoco);
  show(ptr,items, n);
  if(up) ptr--;
  if(down) ptr--;
  ptr = ptr%n;
  if(clk) {
    if(ptr==0) loco = items[0];
    if(ptr==1) loco = items[1];
    if(ptr==2) loco = items[2];
    if(ptr==3) return 1;
    if(ptr>3)  {
      int i = ptr-3;
      f[i] ^= 1;
      items[ptr] = items[ptr]= String("F"+ i) + String( (f[i] ? "on" : "off") );
      return 1;
    }
  }
  return 0;
}
uint8_t menuMain() {
  //P("menuMain");
  static int sub=-1;
  static int ptr=0;
  const int n=4;
  if(up) speed--;
  if(down) speed++;
  if(speed>20) speed=20;
  if(speed<-20) speed=-20;
  if(clk) sub=0;
  if(sub==0) submenu(menuF);
  P("\n\nConnected");
  if(speed>0) { P("\n< "); P(speed); }
  if(speed==0) P("\n  0");
  if(speed<0) { P("\n  "); P(-speed); P(" >"); }
  return 0;
}

void setup() {
  Serial.begin(115200);
  P("\n\nTest menus\n");
  loco = String();
  for(int i=0;i<5;i++) f[i] = false;
  menuMain();
  //while(1){}  
}

void loop() {
  clk=false;
  up=false;
  down=false;
  while(Serial.available()) {
    char c = Serial.read();
    if(c==' ') clk=true;
    else if(c==',') up=true;
    else if(c=='.') down=true;
    else break;
    if(menuMain()) {
      P("\nSelection="); P(selection);
    }
  }
}
