struct packing_buffer pack_me(struct prefetch_queue_m2 pq) {
  struct instruction NO_OP = get_NOP();
  struct packing_buffer pb = {NO_OP, NO_OP};
  //int b_not_packed = 0;
  if (pq.instr1_1.PC == 0 && pq.instr1_2.PC == 0){printf("error in reading into the prefetch queue."); return pb;}
  struct instruction a = pq.instr1_1;
  struct instruction b = pq.instr1_2;

  int not_packable = 0;
  //catch_data_hazard(a,b);

  if (not_packable) {
    //somehow set b_not_packed flag
    if (a.type == ti_STORE || a.type == ti_LOAD) { pb.inst_for_pipe_1 = NO_OP; pb.inst_for_pipe_2 = a; } 
    else { pb.inst_for_pipe_1 = a; pb.inst_for_pipe_2 = NO_OP; }
    return pb;
  }
  //case 1 and b can be packed together and neither are branch or jumps
  if (a.type == ti_NOP)
  {
    if (b.type == ti_STORE || b.type == ti_LOAD) { pb.inst_for_pipe_1 = a; pb.inst_for_pipe_2 = b;}
    else {pb.inst_for_pipe_1 = b;  pb.inst_for_pipe_2 = a; }
  } 
  else if (a.type ==  ti_RTYPE || a.type == ti_ITYPE || a.type == ti_SPECIAL) {
    if (b.type == ti_STORE || b.type == ti_LOAD) {pb.inst_for_pipe_1 = a; pb.inst_for_pipe_2 = b;}
    else {
      pb.inst_for_pipe_1 = a;
      pb.inst_for_pipe_2 = NO_OP;
     // b_not_packed = 1;
    }
  }
  else if (a.type == ti_STORE || a.type == ti_LOAD) {
      if (b.type != ti_STORE && b.type != ti_LOAD) {pb.inst_for_pipe_1 = b, pb.inst_for_pipe_2 = a;}
      else {
        pb.inst_for_pipe_1 = NO_OP;
        pb.inst_for_pipe_2 = a;
        //b_not_packed = 1;
      } 
  }
  else {
     pb.inst_for_pipe_1 = a;
     pb.inst_for_pipe_2 = NO_OP;
     //b_not_packed = 1;
  }

  //if (b_not_packed){
    //somehow set a flag
    
  //}
  return pb;

}

