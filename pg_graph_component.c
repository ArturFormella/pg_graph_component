/*
Author: Artur Formella
*/

#include <postgres.h>
#include <utils/lsyscache.h>
#include <utils/array.h>
#include "utils/hsearch.h"
#include "funcapi.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#else
#error "PG_MODULE_MAGIC wasn't defined!"
#endif

PG_FUNCTION_INFO_V1(graph_components_final);
PG_FUNCTION_INFO_V1(graph_components_step_arr);
PG_FUNCTION_INFO_V1(get_connected_components);
PG_FUNCTION_INFO_V1(graph_component_combine);

typedef struct VertexHash {
    int id;
} VertexHash;

struct Vertex;

typedef struct Vertex { // wierzchołek
    int id;
    struct Vertex* firstItem;
    struct Vertex* lastItem;
    struct Vertex* nextItem;
} Vertex;

typedef struct GraphComponentState {  // Stan agregacji
    HTAB *vertices;
    Vertex *next;
    HASH_SEQ_STATUS hash_seq;
} GraphComponentState;

void copyLinkedListToStr(Vertex *arr, StringInfo string, int deep) {
  if (arr) {
    appendStringInfo(string, "%d", arr->id);
    if (deep > 10000) {
       elog(ERROR, "DEEP 1000");
       return;
    }
    if (arr->nextItem) {
      appendStringInfoString(string, ",");
      copyLinkedListToStr(arr->nextItem, string, deep+1);
    } 
  }
}

/*
  Łączę 2 zbiory sortując w czasie liniowym względem sumy elementów
  Złożoność: O(M+N).
  Na wejściu mam dwie Linked-List wierzchołków, które chcę w miejscu złączyć i posortować rosnąco wg pola ID.
  Każdy z wierzchołków posiada wskaźnik na pierwszy i ostatni element listy.
  Po złączeniu wszystkie wierzchołki wchodzące w skład listy będą miały wspólny pierwszy i ostatni element.
  Lista zawsze zawiera co najmniej 1 wierzchołek. Wynik zawiera co najmniej dwa wierzchołki.
  Możliwe, że na wejściu dostajemy coś co już jest połączone - gdy posiada ten sam FIRST i LAST. Wtedy nic nie robimy.
  Limitem jest RAM.
*/

void mergeToFirst(Vertex* A, Vertex* B) {
/*
  if (B->firstItem != B->firstItem->firstItem){
    elog(ERROR, "RR firstItem nie");
  }

  StringInfo Astring = makeStringInfo();
  copyLinkedListToStr(A->firstItem, Astring, 1);

  StringInfo Bstring = makeStringInfo();
  copyLinkedListToStr(A->firstItem, Bstring, 1);

  StringInfo LLstring = makeStringInfo();
  copyLinkedListToStr(A->firstItem, LLstring, 1);

  StringInfo RRstring = makeStringInfo();
  copyLinkedListToStr(A->firstItem, RRstring, 1);
*/
/*
  if (LL == RR && LL->id != RR->id) {
    elog(ERROR, "onie");
  }
  if (LL != RR && LL->id == RR->id) {
    elog(ERROR, "onie");
  }
  if (LL->firstItem != LL->firstItem->firstItem) {
    elog(ERROR, "LL firstItem xle");
  }
  if (RR->firstItem != RR->firstItem->firstItem) {
    elog(ERROR, "RR firstItem xle");
  }
  if (A->firstItem != A->firstItem->firstItem) {
    elog(ERROR, "A firstItem xle");
  }
*/
  if (A->firstItem != B->firstItem) { // jeśli zbiory nie są połączone
    // ostatnim będzie największy numerek z końców list
    Vertex* newLast = (A->lastItem->id >= B->lastItem->id) ? A->lastItem : B->lastItem;

    bool side = A->firstItem->id < B->firstItem->id;

    Vertex* LL = side ? A->firstItem : B->firstItem; // niższy numerek
    Vertex* RR = side ? B->firstItem : A->firstItem; // wyższy numerek
    Vertex* newFirst = LL;

    A->firstItem = newFirst;
    B->firstItem = newFirst;

    A->lastItem = newLast;
    B->lastItem = newLast;

    LL->lastItem = newLast;
    RR->lastItem = newLast;

      /*
    if (A->firstItem->nextItem == B) {
      elog(ERROR, "er (%d, %d)\n(A %s)\n(B %s)\nLL  %d first: %d(%d) last %d \nRR %d first: %d(%d) last %d\nLAST: (%d, %d)\nFIRSTLAST: (%d, %d)", 

        A->id,   
        B->id,   

        Astring->data,
        Bstring->data,

        LL->id,
        LL->firstItem->id,
        LL->firstItem->nextItem->id,
        LL->lastItem->id,

        RR->id,
        RR->firstItem->id,
        RR->firstItem->nextItem->id,
        RR->lastItem->id,
        A->lastItem->id,
        B->lastItem->id,
        LL->lastItem->id,
        RR->lastItem->id
        );
    }
    */

    Vertex* prevItem = newFirst;  // ten pierwszy
    Vertex* LLNext = newFirst->nextItem;  // do porównań bierzemy kolejny z LL (bo pierwszy z LL już jest wybrano jako początek listy)
    Vertex* RRNext = RR;

    int deep = 0;
    while (LLNext != NULL || RRNext != NULL) {
      if (deep++ > 200000002) {
        elog(ERROR, "mergeToFirst deep 200000002");
      }
/*
      if (LLNext == RRNext && LLNext != NULL&& RRNext != NULL) {
        elog(ERROR, "LLNext = RRNext");
      }
      if (prevItem == RRNext) {
        elog(ERROR, "prevItem = RRNext\n(%d, %d)\nFirst: (%d, %d)\nA: %s\nB: %s\nLL:%s\nRR:%s",
          A->id, B->id,
          A->firstItem->id,
          B->firstItem->id,
          Astring->data,
          Bstring->data,
          LLstring->data,
          RRstring->data
        ); 
      }
      if (prevItem == LLNext) {    elog(ERROR, "prevItem = LLNext");      }
      if (prevItem == NULL) {        elog(ERROR, "prevItem NULL");      }
*/
      // szukamy mniejszego numeru wierzchołka
      if ( !RRNext || (LLNext && LLNext->id < RRNext->id)) {   // LL jest mniejszy lub nie ma RR więc wybieram LL
        prevItem->nextItem = LLNext;   // dopnij ten wybrany
        prevItem = LLNext;
        LLNext = LLNext->nextItem;

      } else if(RRNext) {   // LL jest większy (lub równy), lub nie ma LL więc wybieram RR
    //    if (prevItem->nextItem == RRNext && RRNext!= NULL) {     elog(ERROR, "nextItem = RRNext");   }
        prevItem->nextItem = RRNext;   // dopnij łańcuch
        prevItem = RRNext;
/*
        if (RRNext == RRNext->nextItem) {     //     elog(ERROR, "RRNext = RRNext");        }
        if (prevItem->nextItem == RRNext->nextItem && prevItem->nextItem!= NULL) {     elog(ERROR, "3 nextItem = RRNext");       }    
        if (prevItem->nextItem == RRNext->nextItem && prevItem->nextItem!= NULL) {     elog(ERROR, "2 nextItem = RRNext");       } 
*/
        RRNext = RRNext->nextItem;
      } else {  // OBA NULL
        prevItem->nextItem = NULL;
        RRNext = NULL;
      }
      prevItem->firstItem = newFirst;
      prevItem->lastItem = newLast;
    }
  }
}
Vertex * addToIndex(int32 vertexId, GraphComponentState* state) {
  VertexHash disKey;
  bool found;
  disKey.id = vertexId;
  Vertex *fromVertex = (Vertex*) hash_search(state->vertices, (void *)&disKey, HASH_ENTER, &found); // szukaj lub wstaw
  if (!found) {  // istniał w indeksie
    fromVertex->id = vertexId;
    fromVertex->nextItem = NULL;
    fromVertex->firstItem = fromVertex;
    fromVertex->lastItem = fromVertex; 
  }
  return fromVertex;
}

/*
  Złożoność O(N * logM)
  M - ilość elementów w array komponentu
*/
Datum
graph_components_step_arr(PG_FUNCTION_ARGS) {
    MemoryContext agg_context;
    GraphComponentState *state;
    ArrayType *currentArray;
    int16 elemTypeWidth;
    bool elemTypeByValue;
    char elemTypeAlignmentCode;
    int currentLength;
    int id;
    int i;
    bool *currentNulls;
    Datum *currentVals;

    if (!AggCheckCallContext(fcinfo, &agg_context)) {
      elog(ERROR, "graph_components_step_arr called in non-aggregate context");
    }

    if (PG_ARGISNULL(0)) {
      state = (GraphComponentState *) MemoryContextAlloc(agg_context, sizeof(GraphComponentState)); // O(logN)
      HASHCTL ctlDistinct;
      memset(&ctlDistinct, 100, sizeof(ctlDistinct));
      ctlDistinct.keysize = sizeof(VertexHash);
      ctlDistinct.entrysize = sizeof(Vertex);
      ctlDistinct.hcxt = agg_context;
      state->vertices = hash_create("Graph vertex", 48, &ctlDistinct, HASH_ELEM | HASH_BLOBS);
      state->next = NULL;

    } else {
      state = (GraphComponentState *) PG_GETARG_POINTER(0);
    }

    currentArray = PG_GETARG_ARRAYTYPE_P(1);
    get_typlenbyvalalign(INT4OID, &elemTypeWidth, &elemTypeByValue, &elemTypeAlignmentCode);
    deconstruct_array(currentArray, INT4OID, elemTypeWidth, elemTypeByValue, elemTypeAlignmentCode, &currentVals, &currentNulls, &currentLength); // O(N)

    Vertex *prevVertex;
    Vertex *currentVertex; 

    for (i = 0; i < currentLength; i++) { // O(N * logN)
      id = DatumGetInt32(currentVals[i]);
      currentVertex = addToIndex(id, state);
      if (i > 0) {   // połącz z poprzednim w arr
        mergeToFirst(prevVertex, currentVertex);
      }
      prevVertex = currentVertex;
    }
    PG_RETURN_POINTER(state);
}

/* Wypełnij array danymi z LinkedList. Złożoność O(N) */
void copyLinkedListToArray(Vertex *arr, Datum* output, int deep, int max) {
  if (deep >= max) {
    return;
  }
  if (arr) {
    output[deep] = Int32GetDatum(arr->id);
    if (arr->nextItem && arr->nextItem != arr) {
      copyLinkedListToArray(arr->nextItem, output, deep+1, max);
    }
  }
}

/* Policz ilość elementów. Złożoność O(logN) */
int countLinkedList(Vertex *arr, int deep) {

  if (arr == NULL) {
    return 0;
  }
  if (arr->nextItem == NULL || arr->nextItem == arr) {
    return 1;
  }
  if (deep >= 15000321) {
    return 0;
  }
  return 1 + countLinkedList(arr->nextItem, ++deep);
}

/*
  Datum
  graph_component_combine(PG_FUNCTION_ARGS)
  {
      GraphComponentState *state1;
      GraphComponentState *state2;
      MemoryContext agg_context;
      MemoryContext old_context;
      Vertex *s, *s2;

      HASH_SEQ_STATUS scan_status;
      Vertex *entry;
      Vertex *currentVertex;
      bool found;

      if (!AggCheckCallContext(fcinfo, &agg_context)) {
        elog(ERROR, "graph_component_combine called in non-aggregate context");
      }

      state1 = PG_ARGISNULL(0) ? NULL : (GraphComponentState *) PG_GETARG_POINTER(0);
      state2 = PG_ARGISNULL(1) ? NULL : (GraphComponentState *) PG_GETARG_POINTER(1);

      if (state2 == NULL){
          int size = hash_get_num_entries(state1->vertices);
          elog(WARNING, "connect only A: %d", size);
          PG_RETURN_POINTER(state1);
      }
      old_context = MemoryContextSwitchTo(agg_context);
      if (state1 == NULL)
      {
          state1 = (GraphComponentState *) MemoryContextAlloc(agg_context, sizeof(GraphComponentState));
          HASHCTL ctlDistinct;
          memset(&ctlDistinct, 100, sizeof(ctlDistinct));
          ctlDistinct.keysize = sizeof(VertexHash);
          ctlDistinct.entrysize = sizeof(Vertex);
          ctlDistinct.hcxt = agg_context;
          state1->vertices = hash_create("Graph vertex", 48, &ctlDistinct, HASH_ELEM | HASH_BLOBS);
          state1->next = NULL;

          hash_seq_init(&scan_status, state2->vertices);
          while ((entry = (Vertex*) hash_seq_search(&scan_status)) != NULL) {
              currentVertex = addToIndex(id, state1);
              // kopiuj połączenia z entry do currentVertex

              Vertex* link = currentVertex->firstItem;
              while (link != NULL) {
                link->lastItem = newLast;
                link->firstItem = newFirst;

                link = link->nextItem;
              }  
          }

          for(s2=state2->keys; s2 != NULL; s2=(hashMap*)(s2->hh.next)) {
            elog(WARNING, "  user id %d: value %d", s2->id, s2->value);
            s = (hashMap*)malloc(sizeof(hashMap));
            if(s == NULL){
               elog(ERROR, "malloc error!"); 
            }else{
              s->value = s2->value;
              s->connected = NULL;
              s->id = s2->id;
              HASH_ADD_INT( state1->keys, id, s );
            }
          }
          MemoryContextSwitchTo(old_context);

          PG_RETURN_POINTER(state1);
      } else {
        // state1->inputElementType = state2->inputElementType;
        // merge the two arrays 
        for(s2=state2->keys; s2 != NULL; s2=(hashMap*)(s2->hh.next)) {
            //  elog(WARNING, "  user id %d: value %d", s2->id, s2->value);
            searchForKey = s2->id;
            HASH_FIND_INT( state1->keys, &searchForKey, s );// each element is an ID of tag, add +1 for every ID

            if(s == NULL){
              s = (hashMap*)malloc(sizeof(hashMap));
              if(s == NULL){
                 elog(ERROR, "malloc error!"); 
              }else{
                s->value = s2->value;
                s->id = searchForKey;
                s->connected = NULL;
                HASH_ADD_INT( state1->keys, id, s );
              }
            }else{
              s->value += s2->value;
            }
        }
    //    elog(WARNING, "connect %d + %d = %d", s1, HASH_COUNT(state2->keys), HASH_COUNT(state1->keys) );
        MemoryContextSwitchTo(old_context);

        PG_RETURN_POINTER(state1);
      }
  }
*/

Datum graph_components_final(PG_FUNCTION_ARGS) {
  GraphComponentState *state = (GraphComponentState *) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(state);
}

/*
  Funkcja rozpakowująca dane do wierszy. 
  Złożoność O(N*logN)
*/
Datum
get_connected_components(PG_FUNCTION_ARGS) {
    GraphComponentState *state = (GraphComponentState *) PG_GETARG_POINTER(0);
    FuncCallContext *funcctx;
    MemoryContext oldcontext;
    ArrayType  * result;
    Vertex *vertexEntry;
        
    if (SRF_IS_FIRSTCALL()) {
        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
        funcctx->user_fctx = (void *) state;
        funcctx->max_calls = hash_get_num_entries(state->vertices);
        hash_seq_init(&state->hash_seq, state->vertices);

        MemoryContextSwitchTo(oldcontext);
    }

    funcctx = SRF_PERCALL_SETUP();
    GraphComponentState *input_text = (GraphComponentState *) funcctx->user_fctx;
 
    if (funcctx->call_cntr < funcctx->max_calls) {
      do {
        vertexEntry = (Vertex *) hash_seq_search(&input_text->hash_seq);    // O(logN)
        if (!vertexEntry) {
          //    hash_seq_term(&input_text->hash_seq);
          SRF_RETURN_DONE(funcctx);
          break;
        }
        // pomijamy już wypisane komponenty
      } while (vertexEntry && (vertexEntry->firstItem->nextItem == vertexEntry->firstItem) );   // O(N)
      if (vertexEntry) {
        int size = countLinkedList(vertexEntry->firstItem, 0);
        Datum *output = (Datum*) palloc(size * sizeof(Datum));
        copyLinkedListToArray(vertexEntry->firstItem, output, 0, size);
        result = construct_array(output, size, INT4OID, 4, true, 'i');

        pfree(output);

        // zaznacz żeby już tego wiersza nie wypuszczać
        vertexEntry->firstItem->nextItem = vertexEntry->firstItem;

        SRF_RETURN_NEXT(funcctx, PointerGetDatum(result));
      }
      hash_seq_term(&input_text->hash_seq);

      SRF_RETURN_DONE(funcctx);
    } else {
      hash_seq_term(&input_text->hash_seq);

      SRF_RETURN_DONE(funcctx);
    }
}
