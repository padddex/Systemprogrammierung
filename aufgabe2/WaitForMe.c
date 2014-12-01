/* Einfaches Sleep-Programm zum Testen von Ein-/Ausgabeumleitung
 * Stefan Gast, 2014
 */

#include <stdio.h>
#include <unistd.h>

int main(void)
{
        unsigned seconds;

        if(scanf("%u", &seconds) != 1)
        {
                if(ferror(stdin))
                {
                        perror("error reading from stdin");
                        return 1;
                }
                else
                {
                        fputs("invalid number of seconds!\n", stderr);
                        return 2;
                }
        }

        fprintf(stderr, "sleeping for %u seconds...\n", seconds);
        sleep(seconds);
        puts("Wake up!");

        return 0;
}
