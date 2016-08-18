
MEMORY
{
  ram  : ORIGIN = 0x06000000, LENGTH = 1024M

}

SECTIONS
{
    .text :
    {
        KEEP (*(bootwrapper)) ;

        *(.text .text*) ;
        *(.glue_7t) ;
        *(.glue_7) ;
        *(.gnu.linkonce.t*)

        *(.dtors);
        LONG (0);

    } > ram

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)

    } > ram

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > ram
    __exidx_end = .;

    .rodata :
    {
        *(.rodata*) ;

    } > ram

    fit_image :
	{
	   . = ALIGN(8) ;
	    _image_start = .;
	   KEEP(*(fit_image fit_image*));
	   _image_end = .;

	} > ram

    .data :
    {
	. = ALIGN(4) ;
        *(.data) ;
        *(.data*) ;
        *(.gnu.linkonce.d*)
        . = ALIGN(4) ;
    } > ram

    .heap :
    {
        . = ALIGN(8) ;
        _heap_start = .;

        . += 0x100000;

        . = ALIGN(8) ;
        _heap_end = .;
    } > ram


    .bss (NOLOAD) :
    {
        . = ALIGN(8) ;
        _bss_start = . ;
        *(.bss) ;
        *(.bss*) ;
        *(COMMON) ;
        . = ALIGN(8) ;
        _bss_end = . ;

    } > ram

	__bootwrapper_end = .;
}
