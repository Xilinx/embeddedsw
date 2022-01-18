 /*
 * This file originated from Dhrystone github:
 * https://github.com/Keith-S-Thompson/dhrystone/blob/master/v2.1/
 * This file did not contain a copyright notice or license text.
 * Accordingly, this file is partially authored by an unknown author or authors
 * who may hold the copyright(s) to code in this file. Therefore, use of the
 * non-Xilinx code in this file is at the end users sole risk, and Xilinx
 * does not grant to end user and rights or licenses to any non-Xilinx code in
 * this file.
*/

/*
 ****************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *
 *  Version:    C, Version 2.1
 *
 *  File:       dhry_1.c (part 2 of 3)
 *
 *  Date:       May 25, 1988
 *
 *  Author:     Reinhold P. Weicker
 *
 ****************************************************************************
 */

#include "dhry.h"

#if defined (__MICROBLAZE__)
static void MB_StartAxiTimer(void)
{
	u32 ControlStatusReg;

	/*  Checking if the timer is enabled  */
	if(Xil_In32(MB_AXITIMER_BASEADDR + MB_AXITIMER_TCSR0_OFFSET) &&
			    MB_AXITIMER_CSR_ENABLE_TMR_MASK)
	{
		return;
	}
	/*
	 * Read the current register contents such that only the necessary bits
	 * of the register are modified in the following operations
	 */
	ControlStatusReg = Xil_In32(MB_AXITIMER_BASEADDR +
							    MB_AXITIMER_TCSR0_OFFSET);
	/*
	 * Remove the reset condition such that the timer counter starts running
	 * with the value loaded from the compare register
	 */
	Xil_Out32((MB_AXITIMER_BASEADDR + MB_AXITIMER_TCSR0_OFFSET),
			  (ControlStatusReg | MB_AXITIMER_CSR_ENABLE_TMR_MASK |
			   MB_AXITIMER_CSR_AUTO_RELOAD_MASK));
}

void XTime_GetTime(XTime *time_val)
{
	*time_val = Xil_In32((MB_AXITIMER_BASEADDR) +
						 (MB_AXITIMER_TCR_OFFSET));
}
#endif

/* Porting : Timing functions
 *	How to capture time and convert to seconds must be ported to whatever
 *  is supported by the platform.
 *	e.g. Read value from on board RTC, read value from cpu clock cycles,
 *  performance counter etc.
 */
XTime barebones_clock() {
	XTime time_val;
	XTime_GetTime(&time_val);
	return time_val;
}

void print_time(char *label, float time) {
  int ms = (int)(time * 10000.0F + 0.5F);
  xil_printf("%-43s %d.%04d\n", label, ms / 10000, ms % 10000);
}


/* Define target specific global time variables. */
XTime start_time_val;
XTime stop_time_val;
XTime User_Time;
float Final_User_Time;
float Microseconds_per_Run;
float Dhrystones_Per_Second;
float dmips_per_second;
float dmips_per_mhz;


/* Global Variables: */

Rec_Pointer     Ptr_Glob, Next_Ptr_Glob;
int             Int_Glob;
Boolean         Bool_Glob;
char            Ch_1_Glob, Ch_2_Glob;
int             Arr_1_Glob [50];
int             Arr_2_Glob [50] [50];

Enumeration Func_1 ();
/* forward declaration necessary since Enumeration may not simply be int */
extern Boolean Func_2 (Str_30  Str_1_Par_Ref, Str_30  Str_2_Par_Ref);
extern int Proc_6(Enumeration  Enum_Val_Par, Enumeration *Enum_Ref_Par);
extern int Proc_7(One_Fifty Int_1_Par_Val, One_Fifty  Int_2_Par_Val,
										   One_Fifty *Int_Par_Ref);
extern int Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref,
				  int Int_1_Par_Val, int Int_2_Par_Val);

Rec_Type tmp_var1, tmp_var2;

/* Function : start_time
 *	This function will be called right before starting the timed portion
 *	of the benchmark.
 *	Implementation here is capturing a system timer.
 */
void start_time(void) {
	GETTIME(&start_time_val);
}

/* Function : stop_time
 *	This function will be called right after ending the timed portion
 *  of the benchmark.
 *   Implementation may be capturing a system timer.
 *	or other system parameters - e.g. reading the current value of cpu cycles counter.
 */

void stop_time(void) {
	GETTIME(&stop_time_val);
}

void Proc_3 (Ptr_Ref_Par)
/******************/
	/* executed once */
	/* Ptr_Ref_Par becomes Ptr_Glob */

Rec_Pointer *Ptr_Ref_Par;
{
	if (Ptr_Glob != Null)
    	/* then, executed */
    	*Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
  	Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
} /* Proc_3 */


void Proc_1 (Ptr_Val_Par)
/******************/
Rec_Pointer Ptr_Val_Par;
	/* executed once */
{
	Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;
                                        /* == Ptr_Glob_Next */
	/* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
	/* corresponds to "rename" in Ada, "with" in Pascal           */

	structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
	Ptr_Val_Par->variant.var_1.Int_Comp = 5;
	Next_Record->variant.var_1.Int_Comp
        = Ptr_Val_Par->variant.var_1.Int_Comp;
	Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
	Proc_3 (&Next_Record->Ptr_Comp);
	/* Ptr_Val_Par->Ptr_Comp->Ptr_Comp
                        == Ptr_Glob->Ptr_Comp */
	if (Next_Record->Discr == Ident_1){
	/* then, executed */
		Next_Record->variant.var_1.Int_Comp = 6;
		Proc_6 (Ptr_Val_Par->variant.var_1.Enum_Comp,
		&Next_Record->variant.var_1.Enum_Comp);
		Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
		Proc_7 (Next_Record->variant.var_1.Int_Comp, 10,
		&Next_Record->variant.var_1.Int_Comp);
	}
	else /* not executed */
    		structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
} /* Proc_1 */


void Proc_2 (Int_Par_Ref)
/******************/
	/* executed once */
	/* *Int_Par_Ref == 1, becomes 4 */

One_Fifty   *Int_Par_Ref;
{
	One_Fifty  Int_Loc;
	Enumeration   Enum_Loc;

	Int_Loc = *Int_Par_Ref + 10;
	do /* executed once */
		if (Ch_1_Glob == 'A'){
      		/* then, executed */
      			Int_Loc -= 1;
      			*Int_Par_Ref = Int_Loc - Int_Glob;
      			Enum_Loc = Ident_1;
    		} /* if */
	while (Enum_Loc != Ident_1); /* true */

} /* Proc_2 */


void Proc_4 () /* without parameters */
/*******/
	/* executed once */
{
	Boolean Bool_Loc;

	Bool_Loc = Ch_1_Glob == 'A';
	Bool_Glob = Bool_Loc | Bool_Glob;
	Ch_2_Glob = 'B';
} /* Proc_4 */


void Proc_5 () /* without parameters */
/*******/
	/* executed once */
{
	Ch_1_Glob = 'A';
	Bool_Glob = false;
} /* Proc_5 */


int main ()
/*****/

	/* main program, corresponds to procedures        */
	/* Main and Proc_0 in the Ada version             */
{
	One_Fifty       Int_1_Loc;
	One_Fifty       Int_2_Loc;
	One_Fifty       Int_3_Loc;
	char            Ch_Index;
	Enumeration     Enum_Loc;
	Str_30          Str_1_Loc;
	Str_30          Str_2_Loc;
	int             Run_Index;
	int             Number_Of_Runs;

	/* Initializations */
	init_platform();
	xil_printf("Enter dmips prg\r\n");


	Next_Ptr_Glob = &tmp_var1;
	Ptr_Glob = &tmp_var2;

	Ptr_Glob->Ptr_Comp  = Next_Ptr_Glob;
	Ptr_Glob->Discr      = Ident_1;
	Ptr_Glob->variant.var_1.Enum_Comp     = Ident_3;
	Ptr_Glob->variant.var_1.Int_Comp      = 40;
	strcpy (Ptr_Glob->variant.var_1.Str_Comp,
		"DHRYSTONE PROGRAM, SOME STRING");
	strcpy (Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");

	Arr_2_Glob [8][7] = 10;
	/* Was missing in published program. Without this statement,    */
	/* Arr_2_Glob [8][7] would have an undefined value.             */
	/* Warning: With 16-Bit processors and Number_Of_Runs > 32000,  */
	/* overflow may occur for this array element.                   */

	xil_printf ("\n");
	xil_printf ("Dhrystone Benchmark, Version 2.1 (Language: C)\n");
	xil_printf ("\n");
	{
		xil_printf("Number_Of_Runs set to %d\r\n",ITERATIONS);
		Number_Of_Runs = ITERATIONS;
	}
	xil_printf ("\n");

	xil_printf ("Execution starts, %d runs through Dhrystone\n",
				 Number_Of_Runs);

	/***************/
	/* Start timer */
	/***************/
#if defined (__MICROBLAZE__)
	MB_StartAxiTimer();
#endif
	start_time();

	for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index){
		Proc_5();
		Proc_4();
 		/* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
		Int_1_Loc = 2;
		Int_2_Loc = 3;
		strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
		Enum_Loc = Ident_2;
		Bool_Glob = ! Func_2 (Str_1_Loc, Str_2_Loc);
      		/* Bool_Glob == 1 */
    		while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */ {
      			Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
        		/* Int_3_Loc == 7 */
      			Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
        		/* Int_3_Loc == 7 */
      			Int_1_Loc += 1;
    		} /* while */
      		/* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
    		Proc_8 (Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
      		/* Int_Glob == 5 */
    		Proc_1 (Ptr_Glob);
    		for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index){
 		/* loop body executed twice */
      			if (Enum_Loc == Func_1 (Ch_Index, 'C')){
          		/* then, not executed */
        			Proc_6 (Ident_1, &Enum_Loc);
        			strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
        			Int_2_Loc = Run_Index;
        			Int_Glob = Run_Index;
        		}
    		}
      		/* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
		Int_2_Loc = Int_2_Loc * Int_1_Loc;
		Int_1_Loc = Int_2_Loc / Int_3_Loc;
		Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
		/* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
		Proc_2 (&Int_1_Loc);
 		/* Int_1_Loc == 5 */
	} /* loop "for Run_Index" */

	/**************/
	/* Stop timer */
	/**************/
	stop_time();

	xil_printf ("Execution ends\n");
	xil_printf ("\n");
	xil_printf ("Final values of the variables used in the benchmark:\n");
	xil_printf ("\n");
	xil_printf ("Int_Glob:            %d\n", Int_Glob);
	xil_printf ("        should be:   %d\n", 5);
	xil_printf ("Bool_Glob:           %d\n", Bool_Glob);
	xil_printf ("        should be:   %d\n", 1);
	xil_printf ("Ch_1_Glob:           %c\n", Ch_1_Glob);
	xil_printf ("        should be:   %c\n", 'A');
	xil_printf ("Ch_2_Glob:           %c\n", Ch_2_Glob);
	xil_printf ("        should be:   %c\n", 'B');
	xil_printf ("Arr_1_Glob[8]:       %d\n", Arr_1_Glob[8]);
	xil_printf ("        should be:   %d\n", 7);
	xil_printf ("Arr_2_Glob[8][7]:    %d\n", Arr_2_Glob[8][7]);
	xil_printf ("        should be:   Number_Of_Runs + 10\n");
	xil_printf ("Ptr_Glob->\n");
	xil_printf ("  Ptr_Comp:          %d\n", (int) Ptr_Glob->Ptr_Comp);
	xil_printf ("        should be:   (implementation-dependent)\n");
	xil_printf ("  Discr:             %d\n", Ptr_Glob->Discr);
	xil_printf ("        should be:   %d\n", 0);
	xil_printf ("  Enum_Comp:         %d\n", Ptr_Glob->variant.var_1.Enum_Comp);
	xil_printf ("        should be:   %d\n", 2);
	xil_printf ("  Int_Comp:          %d\n", Ptr_Glob->variant.var_1.Int_Comp);
	xil_printf ("        should be:   %d\n", 17);
	xil_printf ("  Str_Comp:          %s\n", Ptr_Glob->variant.var_1.Str_Comp);
	xil_printf ("        should be:   DHRYSTONE PROGRAM, SOME STRING\n");
	xil_printf ("Next_Ptr_Glob->\n");
	xil_printf ("  Ptr_Comp:          %d\n", (int) Next_Ptr_Glob->Ptr_Comp);
	xil_printf ("        should be:   (implementation-dependent), same as above\n");
	xil_printf ("  Discr:             %d\n", Next_Ptr_Glob->Discr);
	xil_printf ("        should be:   %d\n", 0);
	xil_printf ("  Enum_Comp:         %d\n",
				Next_Ptr_Glob->variant.var_1.Enum_Comp);
	xil_printf ("        should be:   %d\n", 1);
	xil_printf ("  Int_Comp:          %d\n",
			    Next_Ptr_Glob->variant.var_1.Int_Comp);
	xil_printf ("        should be:   %d\n", 18);
	xil_printf ("  Str_Comp:          %s\n",
				Next_Ptr_Glob->variant.var_1.Str_Comp);
	xil_printf ("        should be:   DHRYSTONE PROGRAM, SOME STRING\n");
	xil_printf ("Int_1_Loc:           %d\n", Int_1_Loc);
	xil_printf ("        should be:   %d\n", 5);
	xil_printf ("Int_2_Loc:           %d\n", Int_2_Loc);
	xil_printf ("        should be:   %d\n", 13);
	xil_printf ("Int_3_Loc:           %d\n", Int_3_Loc);
	xil_printf ("        should be:   %d\n", 7);
	xil_printf ("Enum_Loc:            %d\n", Enum_Loc);
	xil_printf ("        should be:   %d\n", 1);
	xil_printf ("Str_1_Loc:           %s\n", Str_1_Loc);
	xil_printf ("        should be:   DHRYSTONE PROGRAM, 1'ST STRING\n");
	xil_printf ("Str_2_Loc:           %s\n", Str_2_Loc);
	xil_printf ("        should be:   DHRYSTONE PROGRAM, 2'ND STRING\n");
	xil_printf ("\n");


	if (stop_time_val < start_time_val) {
		User_Time = ((-1) - start_time_val) + stop_time_val;
	} else {
		User_Time = stop_time_val - start_time_val;
	}

	if (User_Time < Too_Small_Time){
		xil_printf ("Measured time too small to obtain meaningful results\n");
		xil_printf ("Please increase number of runs\n");
		xil_printf ("\n");
	} else {
		Final_User_Time = (float)User_Time/(float)(COUNTS_PER_SECOND);
		Final_User_Time *= Mic_secs_Per_Second;

		Microseconds_per_Run = (float) Final_User_Time /
							   (float) Number_Of_Runs;
		Dhrystones_Per_Second = ((float) Number_Of_Runs /
								(float) Final_User_Time) * Mic_secs_Per_Second;
		dmips_per_second = (float)Dhrystones_Per_Second/(float)1757;
		dmips_per_mhz = dmips_per_second /
					   ((float)CLOCKS_PER_SEC/Mic_secs_Per_Second);

		print_time ("Microseconds for one run through Dhrystone:",
					Microseconds_per_Run);
		print_time ("Dhrystones per Second:", Dhrystones_Per_Second);
		print_time ("DMIPS/Sec:", dmips_per_second);
		print_time ("DMIPS/MHz:", dmips_per_mhz);
		xil_printf ("\n");
		xil_printf("The Dhrystone App has run successfully\r\n");
	}

	cleanup_platform();
}
