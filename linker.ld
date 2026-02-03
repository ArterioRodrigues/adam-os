/*
 *Starts the code at _start
 * */
ENTRY(_start)

SECTIONS {
  /*
   * The first location 
   */
  . = 0x1000;

  /*
   * This is the text section here all .text section all .o files
   * code instructions for example functions
   */
  .text : {
    *(.text)
  }

  /*
   * This is for initialized global staic variables
   * For example int x= 5;
   */
  .data : {
    *(.data)
  }


  /*
   * This is for uninitialized global/static valriables
   * For example int x;
   */
  .bss : {
    *(.bss)
  }
}
