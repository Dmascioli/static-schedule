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

int branch_prediction_correctness(struct instruction ID_1, struct packing_buffer packer) {
  if (ID_1.type != ti_BRANCH) {return 0;}
  if ((packer.inst_for_pipe_1.PC == ID_1.Addr) || (packer.inst_for_pipe_2.PC == ID_1.Addr)) {return 1;}
  else return 0;
}


int catch_data_hazard(struct instruction a, struct instruction b) //returns 1 if the two instructions cannot be packed together 
{
  //check that the second [b] does not depend on the first

  unsigned char dReg = 'A';
  switch(a.type) {
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
        case ti_ITYPE:  
        case ti_LOAD:
        case ti_JRTYPE:
          dReg = a.dReg;
          break;
      }
  if (dReg == 'A') {return 0;}
   switch(b.type) {
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          if (b.sReg_a == dReg | b.sReg_b == dReg) {return 1; }
          break;
        case ti_ITYPE:
          if (b.sReg_a == dReg) {return 1;}
          break;
        case ti_STORE:
          if (b.sReg_a == dReg) {return 1; }
          break;
        case ti_BRANCH:
          if (dReg == b.sReg_a | dReg == b.sReg_b) {return 1;} 
          break;
        case ti_JRTYPE:
          if (dReg == b.sReg_a) {return 1;}
          break;
      }
       return 0;
  if (a.type != ti_LOAD) {return 0;}
  switch(b.type) {
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          if (b.sReg_a == a.dReg | b.sReg_b == a.dReg) {return 1;}
          break;
        case ti_ITYPE:
          if (b.sReg_a == a.dReg) {return 1;}
          break;
        case ti_STORE:
          if (b.sReg_a == a.dReg) {return 1; }
          break;
        case ti_BRANCH:
          if (a.dReg == b.sReg_a | a.dReg == b.sReg_b) {return 1;} 
          break;
        case ti_JRTYPE:
          if (a.dReg == b.sReg_a) {return 1;}
          break;
      return 0;
  }
}

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
  struct prefetch_queue_m2 pq = {NO_OP, NO_OP, NO_OP, NO_OP};
  struct packing_buffer pb = {NO_OP, NO_OP};

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];
  if (argc == 4) trace_view_on = atoi(argv[3]) ;

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();

  while(1) {

    //printf("\n************* cycle %d ***********8\n", cycle_number);
    if(pq.instr1_1.PC == 0){
      size_1 = trace_get_item(&tr_entry_1); /* put the instruction into a buffer */
      if(size_1) {
        size_2 = trace_get_item(&tr_entry_2);
      }
      else {
        memcpy(tr_entry_2, &NO_OP, sizeof(*tr_entry_2));;
      }
  
    }
    else{
      size_1 = trace_get_item(&tr_entry_1);
    }

    //printf("\ntrace_entry_1 is: %d trace_entry_2 is: %d\n", tr_entry_1->PC, tr_entry_2->PC);

    //printf("\ncycle number %d has trace_1 = %d, trace_2 = %d\n", cycle_number, tr_entry_1->PC, tr_entry_2->PC);
    
    if (!size_1) {memcpy(tr_entry_1, &NO_OP, sizeof(*tr_entry_1));}
    if (!size_1 && flush_counter_1<=0 && flush_counter_2<=0) { 
      //printf("weird!!! what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);      /* no more instructions (instructions) to simulate */
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      break;
    }
    else{              /* move the pipeline forward */
      cycle_number++;
     //printf("pq instr 1 PC: %d\n" ,pq.instr1.PC);

      /* move instructions one stage ahead */
      WB_1 = MEM_1;
      MEM_1 = EX_1;
      EX_1 = ID_1;
      ID_1 = IF_1;

      WB_2 = MEM_2;
      MEM_2 = EX_2;
      EX_2 = ID_2;
      ID_2 = IF_2;

      if(!size_1 && (pq.instr1_1.PC == 0) && (pq.instr1_2.PC == 0)){    /* if no more instructions in trace, reduce flush_counter */
        flush_counter_1--;
        flush_counter_2--;   
      }
      else{   /* put into prefetch queue */
        struct instruction temp1, temp2;
        if(pq.instr1_1.PC == 0 && pq.instr1_2.PC == 0){
           memcpy(&temp1, tr_entry_1, sizeof(temp1));
           memcpy(&temp2, tr_entry_2, sizeof(temp2));
         }
         else if (pq.instr1_2.PC !=0) {
          //printf("weird!!! what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
          memcpy(&temp1, &pq.instr1_1, sizeof(temp1));
          memcpy(&temp2, &pq.instr1_2, sizeof(temp2));
          //printf("\n!!!temp1 is: %d, temp2 is %d\n", temp1.PC, temp2.PC);
          if (pq.instr2_1.PC == 0 && pq.instr2_2.PC == 0) {
           // printf("if** %d, %d", tr_entry_1->PC, tr_entry_2->PC);

            memcpy(&pq.instr1_1, tr_entry_1, sizeof(pq.instr1_1));
            memcpy(&pq.instr1_2, tr_entry_2, sizeof(pq.instr1_2));
          }
          else {
            pq.instr1_1 = pq.instr2_1;
            pq.instr1_2 = pq.instr2_2;
            memcpy(&pq.instr2_1, tr_entry_1, sizeof(pq.instr2_1));
            memcpy(&pq.instr2_2, tr_entry_2, sizeof(pq.instr2_2));
          }
          //printf("weird what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
          //printf("\ntemp1 is: %d, temp2 is %d\n", temp1.PC, temp2.PC);

          memcpy(&temp2, tr_entry_1, sizeof(temp2));
          pq.instr1_1 = NO_OP;
         }
        else {
          temp1 = pq.instr1_1;
          memcpy(&temp2, tr_entry_1, sizeof(temp2));
          pq.instr1_1 = pq.instr1_2;
          pq.instr1_2 = pq.instr2_1;
          pq.instr2_1 = pq.instr2_2;
          pq.instr2_2 = get_NOP();
          if (pq.instr2_1.PC !=0) {memcpy(&pq.instr2_2, tr_entry_2, sizeof(pq.instr2_2));}
          else if (pq.instr1_2.PC !=0) {memcpy(&pq.instr2_1, tr_entry_2, sizeof(pq.instr2_1));}
          else if (pq.instr1_1.PC !=0) {memcpy(&pq.instr1_2, tr_entry_2, sizeof(pq.instr1_2));}
          else {memcpy(&pq.instr1_1, tr_entry_2, sizeof(pq.instr1_1));}
          //printf("\nLoading from PQ what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
        }



      if (catch_data_hazard(temp1, temp2)) {
          if (temp1.type == ti_LOAD | temp1.type == ti_STORE){
            pb.inst_for_pipe_2 = temp1;
            pb.inst_for_pipe_1 = get_NOP();
          }
          else {
            pb.inst_for_pipe_1 = temp1;
            pb.inst_for_pipe_2 = get_NOP();
          }
         // printf("\ntemp1 %d, temp2 %d\n", temp1.PC, temp2.PC);
          pq.instr1_1 = temp2;
       }

       else {
        switch(temp1.type){
          case(ti_BRANCH) :
          case(ti_JTYPE) :
          case(ti_JRTYPE): {
            pb.inst_for_pipe_1 = temp1;
            pq.instr1_1 = temp2;
            pb.inst_for_pipe_2 = NO_OP;
            //printf("\nCASE BRANCH what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
            break;
          }
          case(ti_RTYPE) :
          case(ti_ITYPE) :
          case(ti_SPECIAL) : {
            pb.inst_for_pipe_1 = temp1;
            if(temp2.type == ti_LOAD || temp2.type == ti_STORE){
              pb.inst_for_pipe_2 = temp2;
            }
            else{
              pq.instr1_1 = temp2;
              pb.inst_for_pipe_2 = NO_OP;
            }
            //printf("\nCASE ALU what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
            break;
          }
          case(ti_LOAD) :
          case(ti_STORE) : {
            pb.inst_for_pipe_2 = temp1;
            if((temp2.type == ti_LOAD) || (temp2.type == ti_STORE)){
              pq.instr1_1 = temp2;
              pb.inst_for_pipe_1 = NO_OP;
            }
            else{
              pb.inst_for_pipe_1 = temp2;
            }
            //printf("\nCASE lW/ST what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
            break;
          } 
        }
          
      }
      //printf("\nBEFORE what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
      if (branch_prediction_correctness(ID_1, pb)) {  
          if (pq.instr1_1.PC != 0){
            struct instruction a=pq.instr1_1, b=pq.instr1_2, c=pq.instr2_1, d=pq.instr2_2;
            if (pb.inst_for_pipe_1.PC !=0 && pb.inst_for_pipe_2.PC !=0 && pb.inst_for_pipe_2.PC < pb.inst_for_pipe_1.PC) {
              struct instruction copy = pb.inst_for_pipe_1;
              pb.inst_for_pipe_1 = pb.inst_for_pipe_2;
              pb.inst_for_pipe_2 = copy;
              //printf("first");
            }

            if (pb.inst_for_pipe_1.PC != 0) {
             // printf("first what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
              a = pb.inst_for_pipe_1;
              if (pb.inst_for_pipe_2.PC !=0) {
                b = pb.inst_for_pipe_2;
                c = pq.instr1_1;
                d = pq.instr1_2;
              }
              else {
                b = pq.instr1_1;
                c = pq.instr1_2;
                d = pq.instr2_1;
              }

            }
            else {
              //printf("second what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
              a = pb.inst_for_pipe_2;
              b = pq.instr1_1;
              c = pq.instr1_2;
              d = pq.instr2_1;
            }

            //pq.instr1_1 = pb.inst_for_pipe_1;
            //pq.instr1_2 = pb.inst_for_pipe_2;
            pq.instr1_1 = a;
            pq.instr1_2 = b;
            pq.instr2_1 = c;
            pq.instr2_2 = d;
          }
          else if (pq.instr2_1.PC == 0 && pq.instr2_2.PC == 0) {
           // printf("third what is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);
            pq.instr2_1 = pq.instr1_1;
            pq.instr2_2 = pq.instr1_2;
            pq.instr1_1 = pb.inst_for_pipe_1;
            pq.instr1_2 = pb.inst_for_pipe_2;
          }
          else {
           // printf("\n\nAHHHHH PQ IS FULL OH NO!!!!\n\n");
          }
          pb.inst_for_pipe_1 = get_NOP();
          pb.inst_for_pipe_2 = get_NOP();

      }
        IF_1 = pb.inst_for_pipe_1;
        IF_2 = pb.inst_for_pipe_2;
        //printf("\nwhat just went into IF is: IF_1 %d, IF_2 %d\n", IF_1.PC, IF_2.PC);
        //printf("\nwhat is in the pq is: %d, %d, %d, %d\n", pq.instr1_1.PC, pq.instr1_2.PC, pq.instr2_1.PC, pq.instr2_2.PC);

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

