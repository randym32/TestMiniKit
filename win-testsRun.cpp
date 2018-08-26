#pragma warning(disable : 4075)
typedef void(__cdecl *PF)(void);

// Some on sections (from Microsoft)
//  1. Section names must be 8 characters or less.
//  2. The sections with the same name before the $ are merged into one section.
//     The order that they are merged is determined by sorting the characters
//     after the $.  

#pragma section(".rcmTst$a", read)  
__declspec(allocate(".rcmTst$a")) const PF TestSegStart = (PF)1;
#pragma section(".rcmTst$z", read)  
__declspec(allocate(".rcmTst$z")) const PF TestSegEnd = (PF)1;

/// Run the tests
void testsRun()
{
   for (const PF *x = 1 + &TestSegStart; x < &TestSegEnd; ++x)
      // If zero padded, skip
      if (*x)
      {
         // Call the test procedure
         (*x)();
      }

}
