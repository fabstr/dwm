#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>

#include <X11/Xlib.h>


static Display *dpy;

char* smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

void settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}

char* mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf("%s", buf);
}

void setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char* loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f", avgs[1]);
}

float getbattery() {
	FILE *fd;
	int energy_now, energy_full, voltage_now;

	fd = fopen("/sys/class/power_supply/BAT0/energy_now", "r");
	if(fd == NULL) {
		fprintf(stderr, "Error opening energy_now.\n");
		return -1;
	}
	if (EOF == fscanf(fd, "%d", &energy_now)) return 0.0;
	fclose(fd);

	fd = fopen("/sys/class/power_supply/BAT0/energy_full", "r");
	if(fd == NULL) {
		fprintf(stderr, "Error opening energy_full.\n");
		return -1;
	}
	if (EOF == fscanf(fd, "%d", &energy_full)) return 0.0;
	fclose(fd);


	fd = fopen("/sys/class/power_supply/BAT0/voltage_now", "r");
	if(fd == NULL) {
		fprintf(stderr, "Error opening voltage_now.\n");
		return -1;
	}
	if (EOF == fscanf(fd, "%d", &voltage_now)) return 0.0;
	fclose(fd);


	return ((float)energy_now * 1000 / (float)voltage_now) * 100 / ((float)energy_full * 1000 / (float)voltage_now);
}

float getmemory()
{
	float usage;
	struct sysinfo *info;
	
	info = (struct sysinfo *) malloc(sizeof(struct sysinfo));
	sysinfo(info);

	usage = 1 - (((float) info -> freeram + (float) info -> bufferram)
			/ ((float) info -> totalram));
	usage *= 100;

	free(info);
	return usage;
}

int dwmstatus(void)
{
	char *status;
	char *avgs;
	char *tmsth;
	char *tzstockholm = "Europe/Stockholm";

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(1)) {
		avgs = loadavg();
		tmsth = mktimes("%a %d/%m %H:%M:%S, v.%W", tzstockholm);
		float battery = getbattery();
		float memory = getmemory();
		status = smprintf("L:%s M:%.0f%% B:%.0f%% | %s", avgs, memory, battery, tmsth);
		setstatus(status);
		free(avgs);
		free(tmsth);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}
