#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct BHNode{
	int pid;
	int ex_time;
	int org_ex;
	int arrival;
	int l_arrival;
	double priority;
	int visit_num;
	int wait_time;
	
	int degree;
	struct BHNode *child;
	struct BHNode *parent;
	struct BHNode *sibling;
} BHNode;

typedef struct BinomialHeap {
    BHNode *head;
} BinomialHeap;

typedef struct{
	int pid;
	int e_i;
	int tot_arr;
} prcs;

double get_priiority(int e_cur, int e_max, int arr, int visit_num){
	if(visit_num == 1){
		return (double) e_cur;
	}else{
		double temp = pow((2.0 * e_cur) / (3.0 * e_max), 3.0);
		double c_ei = exp(-temp);
		return (1.0 / c_ei) * e_cur;
	}
}

int isBetter(BHNode *a, BHNode *b){
	if (fabs(a->priority - b->priority) > 0.000001) {
        return (a->priority < b->priority);
    }
	
	return (a->arrival < b->arrival);
}

BHNode *nodeInit(int pid, int e_i, int t_arr, int e_max){
	BHNode *node = (BHNode *)malloc(sizeof(BHNode));
	if(node == NULL) return NULL;
	
	node->pid = pid;
	node->ex_time = e_i;
	node->org_ex = e_i;
	node->arrival = t_arr;
	node->l_arrival = t_arr;
	node->visit_num = 1;
	node->wait_time = 0;
	
	node->priority = get_priiority(e_i, e_max, t_arr, 1);
	
	node->degree = 0;
	node->child = NULL;
	node->parent = NULL;
	node->sibling = NULL;
	return node;
}

BinomialHeap *heapInit(void){
    
	BinomialHeap *heap;

    heap = (BinomialHeap *)malloc(sizeof(BinomialHeap));
    if (heap == NULL)
        return NULL;

    heap->head = NULL;
    return heap;
}

BHNode *heapMerge(BHNode *heap1, BHNode *heap2)
{

    if (heap1 == NULL)
        return heap2;
    if (heap2 == NULL)
        return heap1;
	
	BHNode *head;
	BHNode *tail;
  
    if (heap1->degree <= heap2->degree) {
        head = heap1;
        heap1 = heap1->sibling;
    } else {
        head = heap2;
        heap2 = heap2->sibling;
    }

    tail = head;

    while (heap1 != NULL && heap2 != NULL) {
        if (heap1->degree <= heap2->degree) {
            tail->sibling = heap1;
            heap1 = heap1->sibling;
        } else {
            tail->sibling = heap2;
            heap2 = heap2->sibling;
        }
        tail = tail->sibling;
    }

    tail->sibling = (heap1 != NULL) ? heap1 : heap2;
    return head;
}

BHNode *heapUnion(BHNode *original, BHNode *uni){
	BHNode *new_head;
	
	new_head = heapMerge(original, uni);

	if(new_head == NULL) return NULL;
	
	BHNode *prev = NULL;
	BHNode *aux = new_head;
	BHNode *next = aux->sibling;
	
	while(next != NULL){
		if(aux->degree != next->degree || (next->sibling != NULL && next->sibling->degree == aux->degree)){
			prev = aux;
			aux = next;
		}else{
			if(isBetter(aux, next)){
				aux->sibling = next->sibling;
				next->parent = aux;
				next->sibling = aux->child;
				aux->child = next;
				aux->degree++;
	
			}else{
				if(prev == NULL) new_head = next;
				else prev->sibling = next;
				
				aux->parent = next;
				aux->sibling = next->child;
				next->child = aux;
				next->degree++;
				aux = next;
				
			}
		}
		next = aux->sibling;
	}
	return new_head;
}

void heapRemove(BinomialHeap *heap, BHNode *node, BHNode *before) {
    BHNode *child, *new_head, *next;
    if (node == heap->head) heap->head = node->sibling;
    else if (before != NULL) before->sibling = node->sibling;

    new_head = NULL;
    child = node->child;
    while (child != NULL) {
        next = child->sibling;
        child->sibling = new_head;
        child->parent = NULL;
        new_head = child;
        child = next;
    }
    heap->head = heapUnion(heap->head, new_head);
}

void heapInsert(BinomialHeap *heap, BHNode *node) {
    node->sibling = node->child = node->parent = NULL;
    node->degree = 0;
    heap->head = heapUnion(heap->head, node);
}

BHNode *heapGetMin(BinomialHeap *heap){
	BHNode *min;
	BHNode *bef_min;
	BHNode *curr;
	BHNode *prev;
	
	if(heap->head == NULL) return NULL;
	
	min = heap->head;
	bef_min = NULL;
	prev = min;
	curr = min->sibling;
	
	while(curr != NULL){
		if(isBetter(curr, min)){
			min = curr;
			bef_min = prev;
		}
		prev = curr;
		curr = curr->sibling;
	}
	heapRemove(heap, min, bef_min);
	return min;
}

void heapFree(BinomialHeap *heap)
{
    while (heapGetMin(heap) != NULL);
    free(heap);
}

int printPid (BHNode *node){
	int count = 0;
	while(node != NULL){
		count += printf(", P%d", node->pid);
		
		if(node->child != NULL){
			count += printPid(node->child);
		}
		node = node->sibling;
	}
	return count;
}

void printPrioritiies(BHNode *node){
	while(node!= NULL){
		printf(", P%d:%.4f", node->pid, node->priority);
		if(node->child != NULL){
			printPrioritiies(node->child);
		}
		node = node->sibling;
	}
	
}

double run(prcs *input, int num_prcs, int q){
	int cur_time = 0;
	int complete_num = 0;
	int input_index = 0;
	int e_max = 0;
	int twt = 0;
	
	prcs *input_copy = malloc(sizeof(prcs) * num_prcs);
	int k;
	for(k = 0; k < num_prcs; k++){
		input_copy[k] = input[k];
	}
	
	int i;
	for(i = 0; i<num_prcs; i++){
		if(input_copy[i].e_i >e_max){
			e_max = input_copy[i].e_i;
		}
	}
	
	
	printf("\n--------- Trace for Quantum q=%d ---------\n", q);
	printf("%-4s | %-25s | %s\n", "Time", "Processes ", "Priorities");
	printf("----------------------------------------------------------------\n");
	
	
	BinomialHeap *bh = heapInit();
	BHNode *curr_prcs = NULL;
	
	while(complete_num < num_prcs){
		while(input_index < num_prcs && input_copy[input_index].tot_arr <= cur_time){
			heapInsert(bh, nodeInit(input_copy[input_index].pid, input_copy[input_index].e_i, input_copy[input_index].tot_arr, e_max));
			input_index++;
		}
		
		if(curr_prcs == NULL){
			curr_prcs = heapGetMin(bh);
			if(curr_prcs != NULL){
				curr_prcs->wait_time += (cur_time - curr_prcs->l_arrival);
			}else{
				printf("%-4d | %-25s | %s\n", cur_time, "IDLE", "EMPTY");
				if(input_index < num_prcs) cur_time = input_copy[input_index].tot_arr;
				else cur_time++;
				continue;
			}
		}
		
		int run_time;
		if(curr_prcs->ex_time > q) run_time = q;
		else run_time = curr_prcs->ex_time;
		
		int a;
		for(a=0; a < run_time; a++){
			
			printf("%-4d | ", cur_time);
				
			int char_num = 0;
			char_num += printf("P%d", curr_prcs->pid);
			
			if(bh->head != NULL){
				char_num += printPid(bh->head);
			}
			int width = 25;
			while(char_num < width){
				printf(" ");
				char_num++;
			}
			printf(" | ");
			printf("P%d:%.4f", curr_prcs->pid, curr_prcs->priority);
			if(bh->head != NULL){
				printPrioritiies(bh->head);	
			}
			printf("\n");
			
			cur_time++;
			
			while(input_index < num_prcs && input_copy[input_index].tot_arr <= cur_time ){
				heapInsert(bh, nodeInit(input_copy[input_index].pid, input_copy[input_index].e_i, input_copy[input_index].tot_arr, e_max));
				input_index++;
			}
		}
		
		curr_prcs->ex_time -= run_time;
		
		if(curr_prcs->ex_time > 0){
			curr_prcs->visit_num++;
			curr_prcs->l_arrival = cur_time;
			curr_prcs->priority = get_priiority(curr_prcs->ex_time, e_max, cur_time, curr_prcs->visit_num);
			
			curr_prcs->child = NULL;
			curr_prcs->parent = NULL;
			curr_prcs->sibling = NULL;
			curr_prcs->degree = 0;
			
			bh->head = heapUnion(bh->head, curr_prcs);
			
			curr_prcs = NULL;
			
		}else{
			twt += curr_prcs->wait_time;
			complete_num++;
			free(curr_prcs);
			curr_prcs = NULL;
		}
	}
	heapFree(bh);
	free(input_copy);
	return (double)twt / num_prcs;
}

int main(void){
	prcs *processes = NULL;
	int count = 0;
	int pid, ei, tarr;
	int e_max = 0;
	
	while(scanf("%d %d %d", &pid, &ei, &tarr) == 3){
		processes = realloc(processes, sizeof(prcs) * (count + 1));
		processes[count].pid = pid;
		processes[count].e_i = ei;
		processes[count].tot_arr = tarr;
		if(ei > e_max) e_max = ei;
		count++;
		
	}
	if(count == 0) return 0;
	
	double min_awt = -1.0;
	int best_q = 1;
	
	printf("--------------- Results ----------------\n");
    
	
	int q;
	for(q = 1; q <= e_max; q++){
		double current_awt = run(processes, count, q);
		printf("\t\t- AWT -\n");
		printf("\t\t%.4f\n", current_awt);
		if (min_awt < 0 || current_awt < min_awt){
			min_awt = current_awt;
			best_q = q;
		}
	}
	printf("--------------------------------------------\n");
    printf("Best Quantum Value: %d\n", best_q);
    
    
    
    free(processes);
    return 0;
	
}



