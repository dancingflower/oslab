#include "../include/types.h"
#include "../include/stdio.h"
#include "../include/string.h"
#include "../include/stdarg.h"
#include "../include/error.h"

static const char * const error_string[MAXERROR] =
{
	[E_UNSPECIFIED]	= "unspecified error",
	[E_BAD_ENV]	= "bad environment",
	[E_INVAL]	= "invalid parameter",
	[E_NO_MEM]	= "out of memory",
	[E_NO_FREE_ENV]	= "out of environments",
	[E_FAULT]	= "segmentation fault",
};

static void printnum(unsigned long long num, unsigned base, int width, int padc)
{
	if (num >= base)
		printnum(num / base, base, width - 1, padc);
	else
		while (--width > 0) serial_printc(padc);
	putch("0123456789abcdef"[num % base], putdat);
}

static unsigned long long getuint(va_list *ap, int lflag)
{
	if (lflag >= 2)
		return va_arg(*ap, unsigned long long);
	else 
	if (lflag)
		return va_arg(*ap, unsigned long);
	else
		return va_arg(*ap, unsigned int);
}

static long long getint(va_list *ap, int lflag)
{
	if (lflag >= 2)
		return va_arg(*ap, long long);
	else 
	if (lflag)
		return va_arg(*ap, long);
	else
		return va_arg(*ap, int);
}

void printfmt(const char *fmt, ...);

void vprintfmt(const char *fmt, va_list ap)
{
	register const char *p;
	register int ch, err;
	unsigned long long num;
	int base, lflag, width, precision, altflag;
	char padc;

	while (1) {
		while ((ch = *(unsigned char *) fmt++) != '%')
		{
			if (ch == '\0') return;
			serial_printc(ch);
		}

		padc = ' ';
		width = -1;
		precision = -1;
		lflag = 0;
		altflag = 0;
	reswitch:
		switch (ch = *(unsigned char *) fmt++) 
		{
			case '-':
				padc = '-';
				goto reswitch;

			case '0':
				padc = '0';
				goto reswitch;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				for (precision = 0; ; ++fmt) 
				{
					precision = precision * 10 + ch - '0';
					ch = *fmt;
					if (ch < '0' || ch > '9') break;
				}
				goto process_precision;

			case '*':
				precision = va_arg(ap, int);
				goto process_precision;

			case '.':
				if (width < 0) width = 0;
				goto reswitch;

			case '#':
				altflag = 1;
				goto reswitch;

			process_precision:
				if (width < 0) width = precision, precision = -1;
				goto reswitch;

			case 'l':
				lflag++;
				goto reswitch;

			case 'c':
				serial_printc(va_arg(ap, int));
				break;

			case 'e':
				err = va_arg(ap, int);
				if (err < 0) err = -err;
				if (err >= MAXERROR || (p = error_string[err]) == NULL)
					printfmt("error %d", err);
				else
					printfmt("%s", p);
				break;

			case 's':
				if ((p = va_arg(ap, char *)) == NULL) p = "(null)";
				if (width > 0 && padc != '-')
					for (width -= strnlen(p, precision); width > 0; width--)
						serial_printc(padc);
				for (; (ch = *p++) != '\0' && (precision < 0 || --precision >= 0); width--)
					if (altflag && (ch < ' ' || ch > '~'))
						serial_printc('?');
					else
						serial_printc(ch);
				for (; width > 0; width--)
					serial_printc(' ');
				break;


			case 'd':
				num = getint(&ap, lflag);
				if ((long long) num < 0)
				{
					serial_printc('-');
					num = -(long long) num;
				}
				base = 10;
				goto number;

			case 'u':
				num = getuint(&ap, lflag);
				base = 10;
				goto number;

			case 'o':
				num = getuint(&ap, lflag);
				base = 8;
				goto number;

			case 'p':
				serial_printc('0');
				serial_printc('x');
				num = (unsigned long long)
					(uintptr_t) va_arg(ap, void *);
				base = 16;
				goto number;

			case 'x':
				num = getuint(&ap, lflag);
				base = 16;
			number:
				printnum(num, base, width, padc);
				break;

			case '%':
				serial_printc(ch);
				break;

			default:
				serial_printc('%');
				for (fmt--; fmt[-1] != '%'; fmt--)
				/* do nothing */;
				break;
		}
	}
}

void printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintfmt(putch, putdat, fmt, ap);
	va_end(ap);
}


