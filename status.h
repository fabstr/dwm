#ifndef STATUS_H
#define STATUS_H
char* smprintf(char *fmt, ...);
void settz(char *tzname);
char* mktimes(char *fmt, char *tzname);
void setstatus(char *str);
char* loadavg(void);
float getbattery() ;
float getmemory();
int dwmstatus(void);
#endif // STATUS_H
