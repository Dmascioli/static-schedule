/**************************************************************/
/* CS/COE 1541				 			
   compile with gcc -o pipeline five_stage.c			
   and execute using							
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0  
***************************************************************/

#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "CPU.h" 

int main(int argc, char **argv)
{
  struct instruction *tr_entry_1, *tr_entry_2;
  struct instruction IF_1, ID_1, EX_1, MEM_1, WB_1, IF_2, ID_2, EX_2, MEM_2, WB_2;
  struct instruction NO_OP = get_NOP();
  size_t size_1, size_2;
  char *trace_file_name;
  int trace_view_on = 0;
  int flush_counter_1 = 4, flush_counter_2 = 4; //5 stage pipeline, so we have to move 4 instructions once trace is done
  
  unsigned int cycle_number = 0;

  struct prefetch_queue
    {
        struct instruction instr1_1;    //first instruction ahead for pipeline 1
        struct instruction instr2_1;    //second instruction ahead for pipeline 1
        struct instruction instr1_2;    //first instruction ahead for pipeline 2
        struct instruction instr2_2;    //second instruction ahead for pipeline 2
    } pq;

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];
  if (argc == 3) trace_view_on = atoi(argv[2]) ;

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();

  while(1) {
    size_1 = trace_get_item(&tr_entry_1); /* put the instruction into a buffer */
    if( size_1 ) {
      size_2 = trace_get_item(&tr_entry_2);
    }
    else {
      memcpy(tr_entry_2, &NO_OP, sizeof(*tr_entry_2));;
    }
    
   
    if (!size_1 && flush_counter_1==0 && flush_counter_2==0) {       /* no more instructions (instructions) to simulate */
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      break;
    }
    else{              /* move the pipeline forward */
      cycle_number++;

      /* move instructions one stage ahead */
      WB_1 = MEM_1;
      MEM_1 = EX_1;
      EX_1 = ID_1;
      ID_1 = IF_1;

      WB_2 = MEM_2;
      MEM_2 = EX_2;
      EX_2 = ID_2;
      ID_2 = IF_2;

      if(!size_1){    /* if no more instructions in trace, reduce flush_counter */
        flush_counter_1--;
        flush_counter_2--;   
      }
      else{   /* put into prefetch queue */
        memcpy(&IF_1, tr_entry_1, sizeof(IF_1));
        memcpy(&IF_2, tr_entry_2, sizeof(IF_2));
      }

      //printf("==============================================================================\n");
    }  


    if (trace_view_on && cycle_number>=5) {/* print the executed instruction if trace_view_on=1 */
      printf("[cycle %d] \t", cycle_number);
      switch(WB_1.type) {
        case ti_NOP:
          printf("NOP:\n") ;
          break;
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          printf("RTYPE:") ;
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", WB_1.PC, WB_1.sReg_a, WB_1.sReg_b, WB_1.dReg);
          break;
        case ti_ITYPE:
          printf("ITYPE:") ;
          printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", WB_1.PC, WB_1.sReg_a, WB_1.dReg, WB_1.Addr);
          break;
        case ti_LOAD:
          printf("LOAD:") ;      
          printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", WB_1.PC, WB_1.sReg_a, WB_1.dReg, WB_1.Addr);
          break;
        case ti_STORE:
          printf("STORE:") ;      
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", WB_1.PC, WB_1.sReg_a, WB_1.sReg_b, WB_1.Addr);
          break;
        case ti_BRANCH:
          printf("BRANCH:") ;
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", WB_1.PC, WB_1.sReg_a, WB_1.sReg_b, WB_1.Addr);
          break;
        case ti_JTYPE:
          printf("JTYPE:") ;
          printf(" (PC: %d)(addr: %d)\n", WB_1.PC,WB_1.Addr);
          break;
        case ti_SPECIAL:
          printf("SPECIAL:\n") ;      	
          break;
        case ti_JRTYPE:
          printf("JRTYPE:") ;
          printf(" (PC: %d) (sReg_a: %d)(addr: %d)\n", WB_1.PC, WB_1.dReg, WB_1.Addr);
          break;
      }
      printf("\t\t");
    switch(WB_2.type) {
      
      case ti_NOP:
          printf("NOP:\n") ;
          break;
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          printf("RTYPE:") ;
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", WB_2.PC, WB_2.sReg_a, WB_2.sReg_b, WB_2.dReg);
          break;
        case ti_ITYPE:
          printf("ITYPE:") ;
          printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", WB_2.PC, WB_2.sReg_a, WB_2.dReg, WB_2.Addr);
          break;
        case ti_LOAD:
          printf("LOAD:") ;      
          printf(" (PC: %d)(sReg_a: %d)(dReg: %d)(addr: %d)\n", WB_2.PC, WB_2.sReg_a, WB_2.dReg, WB_2.Addr);
          break;
        case ti_STORE:
          printf("STORE:") ;      
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", WB_2.PC, WB_2.sReg_a, WB_2.sReg_b, WB_2.Addr);
          break;
        case ti_BRANCH:
          printf("BRANCH:") ;
          printf(" (PC: %d)(sReg_a: %d)(sReg_b: %d)(addr: %d)\n", WB_2.PC, WB_2.sReg_a, WB_2.sReg_b, WB_2.Addr);
          break;
        case ti_JTYPE:
          printf("JTYPE:") ;
          printf(" (PC: %d)(addr: %d)\n", WB_2.PC,WB_2.Addr);
          break;
        case ti_SPECIAL:
          printf("SPECIAL:\n") ;        
          break;
        case ti_JRTYPE:
          printf("JRTYPE:") ;
          printf(" (PC: %d) (sReg_a: %d)(addr: %d)\n", WB_2.PC, WB_2.dReg, WB_2.Addr);
          break;
    }
    }
  }

  trace_uninit();

  exit(0);
}

