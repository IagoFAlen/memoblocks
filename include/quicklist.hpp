#pragma once
#include <cstdlib>
#include <iostream>
#include "./layout/status.hpp"

namespace mb {
    typedef struct gap {
        void* initial;
        void* final;
        size_t block_size;

        gap(): initial(NULL), final(NULL), block_size(0) {}

        handle::Status push(void* begin, void* last, size_t size){
            if(size == 0)
                return handle::FAILURE;

            initial = begin;
            final = last;
            block_size = size;

            return handle::SUCCESS;
        }
    } Gap;

    typedef struct pivot_idx {
        size_t value;
        size_t idx;

        pivot_idx(): value(0), idx(0) {}

    } PivotIndex;

    typedef struct memo_gaps {
        Gap* gaps;

        size_t size;
        size_t capacity;

        PivotIndex low;
        PivotIndex mid;
        PivotIndex high;
        PivotIndex pivot;

        memo_gaps(): gaps(nullptr), size(0), capacity(0),
            low(PivotIndex()), mid(PivotIndex()), high(PivotIndex()), pivot(PivotIndex()) {}

        ~memo_gaps() {
            if (gaps) {
                free(gaps);
                gaps = nullptr;
            }
            size = capacity = 0;
            low = mid = high = pivot = PivotIndex();
        }

        handle::Status check(){
            return (gaps && capacity > 0) ? handle::SUCCESS : handle::FAILURE;
        }

        handle::Status resize() {
            if (check() != handle::SUCCESS)
                return handle::FAILURE;

            if (size < capacity)
                return handle::SUCCESS;

            size_t new_capacity = capacity == 0 ? 4 : capacity * 2;

            Gap* new_arr = (Gap*) realloc(gaps, new_capacity * sizeof(Gap));
            if (!new_arr)
                return handle::NULLABLE;

            for (size_t i = capacity; i < new_capacity; i++) {
                new_arr[i] = Gap();
            }

            gaps = new_arr;
            capacity = new_capacity;

            return handle::SUCCESS;
        }

        PivotIndex handle_pivot(PivotIndex& low_ref, PivotIndex& mid_ref, PivotIndex& high_ref, size_t b_size){
            if(b_size < low_ref.value){
                low_ref.value = b_size;
                low_ref.idx = size;
            }
            
            if(b_size > high_ref.value){
                high_ref.value = b_size;
                high_ref.idx = size;
            }
            
            mid_ref.idx = (high_ref.idx + low_ref.idx) / 2;
            mid_ref.value = gaps[mid_ref.idx].block_size;
            size++;
            
            PivotIndex aux[3] = {low_ref, mid_ref, high_ref};

            if (aux[0].value > aux[1].value) std::swap(aux[0], aux[1]);
            if (aux[1].value > aux[2].value) std::swap(aux[1], aux[2]);
            if (aux[0].value > aux[1].value) std::swap(aux[0], aux[1]);

            return aux[1];
        }

        handle::Status push(void* begin, void* last, size_t b_size) {
            if (resize() != handle::SUCCESS)
                return handle::FAILURE;

            if(size == 0)
                low.value = mid.value = high.value = b_size;

            gaps[size].push(begin, last, b_size);

            pivot = handle_pivot(low, mid, high, b_size);
            return handle::SUCCESS; 
        }

        Gap* get(size_t idx) {
            return (idx >= size) ? nullptr : &gaps[idx];
        }

    } MemoGaps;

    typedef struct memo_blocks {
        MemoGaps* map;

        bool check_sorting(MemoGaps* partition){
            for(size_t i = 0; i + 1 < partition->size; i++){
                if(partition->get(i)->block_size > partition->get(i + 1)->block_size)
                    return false;
            }
            return true;
        }

        void rebase(MemoGaps* map, MemoGaps* left, MemoGaps* right){
            for(size_t i = 0; i < left->size; i++){
                map->push(left->get(i)->initial, left->get(i)->final, left->get(i)->block_size);
            }

            for(size_t i = 0; i < right->size; i++){
                map->push(right->get(i)->initial, right->get(i)->final, right->get(i)->block_size);
            }
        }

        void fill_partitions(MemoGaps* map, MemoGaps* left, MemoGaps* right){
            if(map->gaps){
                for(size_t i = 0; i < map->size; i++){
                    if(map->get(i)->block_size < map->pivot.value)
                        left->push(map->get(i)->initial, map->get(i)->final, map->get(i)->block_size);
                    else if(map->get(i)->block_size > map->pivot.value)
                        right->push(map->get(i)->initial, map->get(i)->final, map->get(i)->block_size);
                    else {
                        left->push(map->get(i)->initial, map->get(i)->final, map->get(i)->block_size);
                    }
                }
            }
        }

        void quicksort(MemoGaps* map){
            if(map->size <= 1) return;

            MemoGaps* left = new MemoGaps();
            MemoGaps* right = new MemoGaps();

            fill_partitions(map, left, right);

            if(!check_sorting(left))
                quicksort(left);
            
            if(!check_sorting(right))
                quicksort(right);

            rebase(map, left, right);

            delete left;
            delete right;

            return;
        }

    } MemoBlocks;

}
