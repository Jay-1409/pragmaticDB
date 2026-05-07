# Buffer Pool Manager TODOs

This file defines the step-by-step plan for implementing the buffer pool manager.

## 1. Define data structures and core types
- Define `page_id_t`, `frame_id_t`, `INVALID_PAGE_ID`, and `PAGE_SIZE`.
- Define a `Page` struct with:
  - `page_id` (current page id for this frame)
  - `pin_count`
  - `is_dirty`
  - `data[PAGE_SIZE]`
- Define `BufferPoolManager` member fields:
  - `pool_size`
  - `DiskManager *disk_manager`
  - `std::vector<Page> pages`
  - `std::unordered_map<page_id_t, frame_id_t> page_table`
  - `std::deque<frame_id_t> free_list`
  - replacement policy structure (simple LRU or FIFO)
  - `page_id_t next_page_id`

## 2. Implement constructor
- Initialize all pages in `pages`.
- Set every page to `INVALID_PAGE_ID`, `pin_count=0`, `is_dirty=false`.
- Push all frame ids into `free_list`.
- Clear `page_table`.
- Initialize replacement structure as empty.
- Initialize `next_page_id` to 0.

## 3. Implement `FetchPage(page_id)`
- If `page_id` exists in `page_table`:
  - Increment `pin_count`.
  - Remove frame from replacer (not evictable).
  - Return pointer to page.
- Else:
  - Get a frame from `free_list` or the replacer.
  - If no frame, return `nullptr`.
  - If victim frame is dirty, flush it to disk.
  - Remove victim from `page_table`.
  - Read requested page from disk.
  - Set frame metadata (`page_id`, `pin_count=1`, `is_dirty=false`).
  - Insert new mapping into `page_table`.
  - Return pointer to page.

## 4. Implement `UnpinPage(page_id, is_dirty)`
- If `page_id` not in `page_table`, return `false`.
- If `pin_count==0`, return `false`.
- Decrement `pin_count`.
- If `is_dirty==true`, set page dirty.
- If `pin_count` becomes 0, insert frame into replacer.
- Return `true`.

## 5. Implement `NewPage(page_id*)`
- Acquire a free/victim frame (same logic as FetchPage).
- If none available, return `nullptr`.
- Allocate a new `page_id` using `next_page_id`.
- If victim dirty, flush it.
- Remove victim mapping from `page_table`.
- Zero page data.
- Set metadata (`page_id`, `pin_count=1`, `is_dirty=false`).
- Insert into `page_table`.
- Return pointer to page.

## 6. Implement `FlushPage(page_id)` and `FlushAllPages()`
- `FlushPage`:
  - If page not in memory, return `false`.
  - Write page data to disk.
  - Mark `is_dirty=false`.
  - Return `true`.
- `FlushAllPages`:
  - Iterate all pages in `page_table`.
  - Write dirty pages to disk and clear dirty flag.

## 7. Implement `DeletePage(page_id)`
- If page not in memory, return `true` (already gone).
- If `pin_count>0`, return `false`.
- Remove from replacer.
- Erase mapping from `page_table`.
- Reset frame metadata and zero data.
- Add frame back to `free_list`.
- Optionally track freed page ids if you build a free list.

## 8. Implement replacement policy (simple LRU)
- Data structures:
  - `std::list<frame_id_t> lru_list`
  - `std::unordered_map<frame_id_t, list::iterator> lru_pos`
- When a frame becomes evictable, insert into LRU tail.
- When a frame is pinned, remove from LRU.
- Victim is head of LRU list.

## 9. Add a small test in `main.cpp`
- Write a page with known bytes.
- Read it back and verify contents.
- Print success/failure.

## 10. Clean up repo
- Add `.gitignore` to exclude `*.o`, `test`, and `data.db`.
- Remove any tracked build artifacts.
