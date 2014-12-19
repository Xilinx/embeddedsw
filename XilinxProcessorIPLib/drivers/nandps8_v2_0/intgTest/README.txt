NandPs8 Integration test

- On R5 processors, you may get compilation error for bsp stating "undefined reference to `end'"
  Add end = .;  at the end of the linker script to remove this error

  } > ps8_ocm_ram_0_S_AXI_BASEADDR

  _end = .;
  end = .;
}
