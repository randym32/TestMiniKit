
typedef void (*F)(void);
extern int __start___rcmTst  __asm("section$start$__DATA$__rcmTst");
extern int __stop___rcmTst[] __asm("section$end$__DATA$__rcmTst");
/// Run the tests
void testsRun()
{
    for (F* x =  (F*) &__start___rcmTst; x < (F*)&__stop___rcmTst; x++)
        if (*x) (*x)();
}

//https://stackoverflow.com/questions/17669593/how-to-get-a-pointer-to-a-binary-section-in-mac-os-x
