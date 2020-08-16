uint16_t offset;

#define ADD a + b
#define SUB a - b
#define MUL a * b
#define DIV a / b
#define ROUND roundf(a / b)
#define ROUNDA roundf(a)
#define INV 1 / a
#define DBL 2 * a
#define SQRT sqrtf(a)
#define COS cos(a)
#define SIN sin(a)
#define TAN tan(a)
#define HALF (0.5f*a)
#define HALF_FAST ({int32_t result = (*(int32_t*)&a); if((result&0x7f800000)!=0) result-=0x00800000; else result = 0; (*(float*)&result); })

#define PRINT_OP(X) Serial.println(#X)

#define DO_OP(TYPE, MIN, MAX, OP) {\
TYPE a = random(MIN,MAX);TYPE b = random(MIN,MAX);TYPE res = 0;\
TCCR1A = 0;TCCR1B = 0;TCNT1 = 0;TIFR1 = 0;TCCR1B = 1;\
res = OP;\
uint16_t clocks = TCNT1;TCCR1B=0;\
Serial.println(#TYPE);\
PRINT_OP(OP);\
Serial.print("a = ");\
Serial.println(a);\
Serial.print("b = ");\
Serial.println(b);\
if(a<res) \
Serial.print("res > ");\
else \
Serial.print("res < ");\
Serial.println(res);\
Serial.print("Elapsed (clocks): ");\
Serial.println(clocks-offset);\
}

void setup() {
  offset = 0;
  Serial.begin(9600);
  randomSeed(analogRead(0));
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TIFR1 = 0;
  TCCR1B = 1;
  offset = TCNT1;
  TCCR1B=0;

  DO_OP(uint8_t, 0,0xFF>>1,ADD);
  DO_OP(uint8_t, 0,0xFF>>1,SUB);
  DO_OP(uint8_t, 0,0x0F,MUL);
  DO_OP(uint8_t, 0,0x0F,DIV);
  DO_OP(uint16_t, 0,0xFFFF>>1,ADD);
  DO_OP(uint16_t, 0,0xFFFF>>1,SUB);
  DO_OP(uint16_t, 0,0xFF,MUL);
  DO_OP(uint16_t, 0,0xFF,DIV);
  DO_OP(uint32_t, 0,0xFFFFFFFF>>1,ADD);
  DO_OP(uint32_t, 0,0xFFFFFFFF>>1,SUB);
  DO_OP(uint32_t, 0,0xFFFF,MUL);
  DO_OP(uint32_t, 0,0xFFFF,DIV);
  DO_OP(float, 0,10000,ADD);
  DO_OP(float, 0,10000,SUB);
  DO_OP(float, 0,10000,MUL);
  DO_OP(float, 0,10000,DIV);
  DO_OP(float, 0,10000,ROUND);
  DO_OP(float, 0,10000,ROUNDA);
  DO_OP(float, 0,50,INV);
  DO_OP(float, 0,50,DBL);
  DO_OP(float, 0,10000,HALF);
  DO_OP(float, 0,10000,HALF_FAST);
  DO_OP(float, -50000,-10,HALF_FAST);
}

void timeloop(uint16_t loops)
{
  do{
  }while(--loops);
}

void loop() {
  // put your main code here, to run repeatedly:
  timeloop(1000);

}
