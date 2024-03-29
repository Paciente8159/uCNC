uint16_t offset;

typedef union
  {
    float f;
    int32_t i;
    struct
    {
      uint16_t w0;
      int16_t w1;
    };
    struct
    {
      uint8_t b0;
      uint8_t b1;
      uint8_t b2;
      int8_t b3;
    };
    struct
    {
      int32_t mant : 23;
      int32_t expn : 8;
      int32_t sign : 1;
    };
  } flt_t;

#define ADD a + b
#define SUB a - b
#define MUL a * b
#define DIV a / b
#define ROUND roundf(a / b)
#define ROUNDA roundf(a)
#define CAST (int32_t)a
#define INV 1 / (a*0.001f)
#define INV2 ({flt_t res; res.f=a*0.001f; res.i = (0x7eef1aa0 - res.i);res.f;})
#define DBL 2 * a
#define SQRT sqrtf(a)
#define COS cos(a)
#define SIN sin(a)
#define TAN tan(a)
#define TANH ({float k=a*0.001;k-=0.5f;((k / (0.5f + abs(k))) + 0.5f);})
#define SCURVE ({float k=a*0.001;float x=(1.0f/(1.0f+pow(k/(1.0f-k),-2)));x;})
#define BEZIER ({uint32_t k=128*a*0.001;float k2=k*k; float c1=k2-2.5*k; float c2=(c1*3+5); (c2*2*k2*k);})
#define HALF_FAST ({int32_t result = (*(int32_t*)&a); if((result&0x7f800000)!=0) result-=0x00800000; else result = 0; (*(float*)&result); })
#define MUL_124 ((a<<7)-(a<<2))
#define DIV_124 ((a>>7)+(a>>12))
#define FLT_BY4 (       \
  {                            \
    flt_t res;               \
    res.f = (a);             \
    if (res.b3 & 0x7f)       \
      res.i -= 0x01000000; \
    else                     \
      res.i = 0;           \
    res.f;                   \
  })
#define MUL256 a<<16

#define PRINT_OP(X) Serial.println(#X)

#define DO_OP(TYPE, TYPERES, MIN, MAX, OP) {\
TYPE a = random(MIN,MAX);TYPE b = random(MIN,MAX);TYPERES res = 0;\
TCCR1A = 0;TCCR1B = 0;TCNT1 = 0;TIFR1 = 0;TCCR1B = 1;\
res = (TYPERES)OP;\
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

  /*Serial.println("8-bit");
  DO_OP(uint8_t, uint16_t, 0,0xFF,ADD);
  DO_OP(uint8_t, uint16_t, 0,0xFF,SUB);
  DO_OP(uint8_t, uint16_t, 0,0xFF,MUL);
  DO_OP(uint8_t, uint16_t, 0,0xFF,DIV);
  Serial.println("16-bit");
  DO_OP(uint16_t, uint32_t, 0,0xFFFF,ADD);
  DO_OP(uint16_t, uint32_t, 0,0xFFFF,SUB);
  DO_OP(uint16_t, uint32_t, 0,0xFFFF,MUL);
  DO_OP(uint16_t, uint32_t, 0,0xFFFF,DIV);
  DO_OP(uint16_t, uint32_t, 0x0000,0x0FF,MUL_124);
  DO_OP(uint16_t, uint32_t, 0xF000,0xFFFF,DIV_124);
  Serial.println("32-bit");
  DO_OP(uint32_t, uint64_t, 0,0xFFFF,ADD);
  DO_OP(uint32_t, uint64_t, 0,0xFFFF,SUB);
  DO_OP(uint32_t, uint64_t, 0,0xFFFF,MUL);
  DO_OP(uint32_t, uint64_t, 0,0xFFFF,DIV);
  DO_OP(uint32_t, uint64_t, 0,0xFFFF,MUL256);
  Serial.println("64-bit");
  DO_OP(uint64_t, uint64_t, 0,0xFFFF,ADD);
  DO_OP(uint64_t, uint64_t, 0,0xFFFF,SUB);
  DO_OP(uint64_t, uint64_t, 0,0xFFFF,MUL);
  DO_OP(uint64_t, uint64_t, 0,0xFFFF,DIV);*/
  Serial.println("float");
  DO_OP(float, float, 0,1000,TANH);
  DO_OP(float, float, 0,1000,SCURVE);
  DO_OP(float, float, 0,1000,BEZIER);
  DO_OP(float, float, 0,1000,INV);
  DO_OP(float, float, 0,1000,INV2);
  /*Serial.println("testes");
  DO_OP(float, 0,10000,ROUND);
  DO_OP(float, 0,10000,ROUNDA);
  DO_OP(float, 0,50,INV);
  DO_OP(float, 0,50,DBL);
  DO_OP(float, 0,10000,HALF);
  DO_OP(float, 0,10000,HALF_FAST);
  DO_OP(float, -50000,-10,HALF_FAST);*/
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
