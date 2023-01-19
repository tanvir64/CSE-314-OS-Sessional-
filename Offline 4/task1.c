//Task 1: Updating Process Details Viewer
    //adding page table related information
    uint virtual_page_number[1024];
    uint phy_page_number[1024];
    int mapping_count = 0;
    
    cprintf("\nPage Tables:\n");
    cprintf("\tMemory location of page directory = %d\n",V2P(p->pgdir));
    int pd_index;
    for(pd_index = 0;pd_index<NPDENTRIES;pd_index++){
      // cprintf("%d\n",pd_index);
      //if user program have access to the page table entry
      if((PTE_U & PTE_FLAGS(p->pgdir[pd_index])) && (PTE_A & PTE_FLAGS(p->pgdir[pd_index]))){        
        pte_t* pte = (pte_t*)PTE_ADDR(p->pgdir[pd_index]); // retriving the page table entry(32 bits) of the page directory.        
        cprintf("\tpdir PTE %d, %d:\n",pd_index,(uint)pte>>12); //PPN will be the leftmost 20 bits of the page table entry
        cprintf("\t\tMemory location of page table = %x\n",pte); //physical memory address of the page table
        for(int pt_index=0;pt_index<NPTENTRIES;pt_index++){
          pte_t* pte_layer2 = (pte_t*)((pte_t*)P2V(pte))[pt_index]; // retriving the virtual page number of the page table        
          // pte_t* pte_layer2 = (pte_t*)PTE_ADDR(pte[pt_index]);          
          // pte_t* pte_layer2 = (pte_t*)(pte[pt_index]);
          if((PTE_U & PTE_FLAGS(pte_layer2)) && (PTE_A & PTE_FLAGS(pte_layer2))){    //checking the page of the current page's flags                    
            // mapping between virtual and physical pages
            virtual_page_number[mapping_count] = (pd_index << 10) + pt_index; //calculating the virtual page number
            phy_page_number[mapping_count] = (uint)pte_layer2>>12;
            mapping_count++;
            cprintf("\t\tptbl PTE %d, %d, %x\n",pt_index,(uint)pte_layer2>>12,PTE_ADDR(pte_layer2));
          }
        }
      }
      // pd_index++;
    }
    cprintf("Page Mappings:\nVPN -> PPN\n");
    for(int i=0;i<mapping_count;i++){
      cprintf("%d -> %d\n",virtual_page_number[i],phy_page_number[i]);
    }