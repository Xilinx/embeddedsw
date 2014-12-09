NandPs8 Integration test

- On A53 processors use the linker script for DDR region to run the integration test
  With OCM linker script which is the default linker script, the integration test may hang.
  In order to run on OCM, you can use Xil_DCacheDisable() in the main and run the integration test.

- On R5 processors, you may get compilation error for bsp stating "undefined reference to `end'"
  Add end = .;  at the end of the linker script to remove this error

  } > ps8_ocm_ram_0_S_AXI_BASEADDR

  _end = .;
  end = .;
}
