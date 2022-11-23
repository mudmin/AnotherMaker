
/* Copyright (c)1999 Microchip Technology */

/* MODDED by F. Vecoven to accommodate bootloader */

/* MPLAB-C18 startup code, including initialized data */

/* external reference to __init() function */
extern void __init (void);
/* external reference to the user's main routine */
extern void main (void);
/* prototype for the startup function */
void _entry (void);
void _startup (void);
/* prototype for the initialized data setup */
void _do_cinit (void);

extern volatile near unsigned long short TBLPTR;
extern near unsigned FSR0;
extern near char __FPFLAGS;
#define RND 6


#pragma code _startup_scn
void
_startup (void)
{
  _asm
    // Initialize the stack pointer
    lfsr 1, _stack
    lfsr 2, _stack

    clrf TBLPTRU, 0 // 1st silicon doesn't do this on POR

    bcf __FPFLAGS,RND,0 // Initialize rounding flag for floating point libs
    
    _endasm 
    _do_cinit ();

loop:

  // If user defined __init is not found, the one in clib.lib will be used
  __init ();

  // Call the user's main routine
  main ();

  goto loop;
}                               /* end _startup() */

/* MPLAB-C18 initialized data memory support */
/* The linker will populate the _cinit table */
extern far rom struct
{
  unsigned short num_init;
  struct _init_entry
  {
    unsigned long from;
    unsigned long to;
    unsigned long size;
  }
  entries[];
}
_cinit;

#pragma code _cinit_scn
void
_do_cinit (void)
{
  /* we'll make the assumption in the following code that these statics
   * will be allocated into the same bank.
   */
  static short long prom;
  static unsigned short curr_byte;
  static unsigned short curr_entry;
  static short long data_ptr;

  // Initialized data...
  TBLPTR = (short long)&_cinit;
  _asm
    movlb data_ptr
    tblrdpostinc
    movf TABLAT, 0, 0
    movwf curr_entry, 1
    tblrdpostinc
    movf TABLAT, 0, 0
    movwf curr_entry+1, 1
  _endasm
    //while (curr_entry)
    //{
    test:
    _asm
     bnz 3
    tstfsz curr_entry, 1
    bra 1
    _endasm
    goto done;
      /* Count down so we only have to look up the data in _cinit
       * once.
       *
       * At this point we know that TBLPTR points to the top of the current
       * entry in _cinit, so we can just start reading the from, to, and
       * size values.
       */
      _asm
	/* read the source address */
	tblrdpostinc
	movf TABLAT, 0, 0
	movwf prom, 1
	tblrdpostinc
	movf TABLAT, 0, 0
	movwf prom+1, 1
	tblrdpostinc
	movf TABLAT, 0, 0
	movwf prom+2, 1
	/* skip a byte since it's stored as a 32bit int */
	tblrdpostinc
	/* read the destination address directly into FSR0 */
	tblrdpostinc
	movf TABLAT, 0, 0
	movwf FSR0L, 0
	tblrdpostinc
	movf TABLAT, 0, 0
	movwf FSR0H, 0
	/* skip two bytes since it's stored as a 32bit int */
	tblrdpostinc
	tblrdpostinc
	/* read the destination address directly into FSR0 */
	tblrdpostinc
	movf TABLAT, 0, 0
	movwf curr_byte, 1
	tblrdpostinc
	movf TABLAT, 0, 0
	movwf curr_byte+1, 1
	/* skip two bytes since it's stored as a 32bit int */
	tblrdpostinc
	tblrdpostinc
      _endasm  
      //prom = data_ptr->from;
      //FSR0 = data_ptr->to;
      //curr_byte = (unsigned short) data_ptr->size;
      /* the table pointer now points to the next entry. Save it
       * off since we'll be using the table pointer to do the copying
       * for the entry.
       */
      data_ptr = TBLPTR;
      
      /* now assign the source address to the table pointer */
      TBLPTR = prom;

      /* do the copy loop */
      _asm
        // determine if we have any more bytes to copy
	movlb curr_byte
	movf curr_byte, 1, 1
copy_loop:
	bnz 2 // copy_one_byte
	movf curr_byte + 1, 1, 1
	bz 7 // done_copying

copy_one_byte:
	tblrdpostinc
	movf TABLAT, 0, 0
	movwf POSTINC0, 0

	// decrement byte counter
	decf curr_byte, 1, 1
	bc -8 // copy_loop
        decf curr_byte + 1, 1, 1
	bra -7 // copy_one_byte

done_copying:

      _endasm
      /* restore the table pointer for the next entry */
      TBLPTR = data_ptr;
      /* next entry... */
      curr_entry--;
      goto test;
done:
;
}
