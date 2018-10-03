int branch_prediction_correctness(struct instruction ID_1, struct packing_buffer packer){
  if (ID_1.type != ti_BRANCH) {return 0;}
  if (packer.inst_for_pipe_1.PC == ID_1.Addr | packer.inst_for_pipe_2 == ID_1.Addr) {return 1;}
  else return 0;
}

struct packing_buffer pack_me(struct prefetch_queue pq) {
  struct packing_buffer pb = {NO_OP, NO_OP};
  //int b_not_packed = 0;
  if (pq.instr1_1.PC == 0 && pq.instr1_2.PC == 0){printf("error in reading into the prefetch queue.") return pb;}
  struct instruction a = pq.instr1_1;
  struct instruction b = pq.instr1_2;

  int not_packable = catch_data_hazard(a,b);

  if (not_packable){
    //somehow set b_not_packed flag
    if (a.type==ti_STORE | a.type == ti_LOAD) {pb = {get_NOP(), a};} 
    else {pb = {a, get_NOP()};}
    return pb;
  }
  //case 1 and b can be packed together and neither are branch or jumps
  if (a.type == ti_NOP)
  {
    if (b.type == ti_STORE | b.type == ti_LOAD) {pb = {a,b};}
    else {pb = {b, a};}
  } 
  else if (a.type ==  ti_RTYPE | a.type == ti_ITYPE | a.type == ti_SPECIAL) {
    if (b.type == ti_STORE | b.type == ti_LOAD) {pb = {a,b};}
    else {
      pb = {a, get_NOP()};
     // b_not_packed = 1;
    }
  }
  else if (a.type == ti_STORE | a.type == ti_LOAD) {
      if (b.type != ti_STORE && b.type != ti_LOAD) {pb = {b, a};}
      else {
        pb = {get_NOP(), a};
        //b_not_packed = 1;
      } 
  }
  else {
     pb = {a, get_NOP()};
     //b_not_packed = 1;
  }

  //if (b_not_packed){
    //somehow set a flag
    
  //}
  return pb;

}


int catch_data_hazard(struct instruction a, struct instruction b) //returns 1 if the two instructions cannot be packed together 
{
  //check that the second [b] does not depend on the first

  unsigned char dReg = 'A';
  switch(a.type) {
        case ti_RTYPE: /* registers are translated for printing by subtracting offset  */
          dReg = a.dReg;
          break;
        case ti_ITYPE:
          dReg = a.dReg;
          break;
        case ti_LOAD:
          dReg = a.dReg;
          break;
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
        case ti_LOAD:
          if (MEM.dReg == b.sReg_a) {return 1;}
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
}
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
