#include "manager/record_manager.h"
#include "ds/page_header.h"
#include "ds/slot.h"
#include <cstring>
#include <limits>

namespace {
bool HasEnoughSpaceForInsert(Page* page, uint16_t tuple_size) {
	PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
	uint16_t required_space = static_cast<uint16_t>(tuple_size + sizeof(Slot));
	uint16_t available_space = static_cast<uint16_t>((header->free_space_offset + 1) -
		(sizeof(PageHeader) + header->slot_count * sizeof(Slot)));
	return required_space <= available_space;
}
}

RecordManager::RecordManager(const std::string& filename)
	: disk_manager_(filename), buffer_pool_manager_(kDefaultPoolSize, &disk_manager_) {}

bool RecordManager::Get(const RecordId& rid, char* data, uint16_t* size) {
	if (rid.page_id == INVALID_PAGE_ID || data == nullptr || size == nullptr) {
		return false;
	}
	Page* page = buffer_pool_manager_.FetchPage(rid.page_id);
	if (!page) {
		return false;
	}
	PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
	if (rid.slot_id >= header->slot_count) {
		buffer_pool_manager_.UnpinPage(rid.page_id, false);
		return false;
	}
	Slot* slot = reinterpret_cast<Slot*>(page->data + sizeof(PageHeader) +
		rid.slot_id * sizeof(Slot));
	if (slot->length == 0) {
		buffer_pool_manager_.UnpinPage(rid.page_id, false);
		return false;
	}
	bool ok = page_data_manager_.GetTuple(page, rid.slot_id, data, size);
	buffer_pool_manager_.UnpinPage(rid.page_id, false);
	return ok;
}

RecordId RecordManager::Insert(const char* data, size_t size) {
	RecordId invalid{INVALID_PAGE_ID, 0};
	if (data == nullptr || size == 0 || size > std::numeric_limits<uint16_t>::max()) {
		return invalid;
	}
	const uint16_t tuple_size = static_cast<uint16_t>(size);
	for (page_id_t page_id : page_ids_) {
		Page* page = buffer_pool_manager_.FetchPage(page_id);
		if (!page) {
			continue;
		}
		if (HasEnoughSpaceForInsert(page, tuple_size)) {
			uint16_t slot_id = 0;
			bool ok = page_data_manager_.InsertTuple(page, data, tuple_size, &slot_id);
			buffer_pool_manager_.UnpinPage(page_id, ok);
			if (ok) {
				return RecordId{page_id, slot_id};
			}
		} else {
			buffer_pool_manager_.UnpinPage(page_id, false);
		}
	}
	for (page_id_t page_id : page_ids_) {
		Page* page = buffer_pool_manager_.FetchPage(page_id);
		if (!page) {
			continue;
		}
		page_data_manager_.CompactOnePage(page);
		if (HasEnoughSpaceForInsert(page, tuple_size)) {
			uint16_t slot_id = 0;
			bool ok = page_data_manager_.InsertTuple(page, data, tuple_size, &slot_id);
			buffer_pool_manager_.UnpinPage(page_id, ok);
			if (ok) {
				return RecordId{page_id, slot_id};
			}
			continue;
		}
		buffer_pool_manager_.UnpinPage(page_id, true);
	}
	page_id_t new_page_id = INVALID_PAGE_ID;
	Page* new_page = buffer_pool_manager_.NewPage(&new_page_id);
	if (!new_page) {
		return invalid;
	}
	page_data_manager_.InitializePage(new_page, new_page_id);
	uint16_t slot_id = 0;
	bool ok = page_data_manager_.InsertTuple(new_page, data, tuple_size, &slot_id);
	buffer_pool_manager_.UnpinPage(new_page_id, ok);
	if (!ok) {
		return invalid;
	}
	page_ids_.push_back(new_page_id);
	return RecordId{new_page_id, slot_id};
}

bool RecordManager::Delete(RecordId r) {
	if (r.page_id == INVALID_PAGE_ID) {
		return false;
	}
	Page* page = buffer_pool_manager_.FetchPage(r.page_id);
	if (!page) {
		return false;
	}
	bool ok = page_data_manager_.DeleteTuple(page, r.slot_id);
	buffer_pool_manager_.UnpinPage(r.page_id, ok);
	return ok;
}

bool RecordManager::Update(const RecordId& rid, const char* data, size_t size) {
	if (rid.page_id == INVALID_PAGE_ID || data == nullptr || size == 0 ||
		size > std::numeric_limits<uint16_t>::max()) {
		return false;
	}

	Page* page = buffer_pool_manager_.FetchPage(rid.page_id);
	if (!page) {
		return false;
	}

	PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
	if (rid.slot_id >= header->slot_count) {
		buffer_pool_manager_.UnpinPage(rid.page_id, false);
		return false;
	}

	Slot* slot = reinterpret_cast<Slot*>(page->data + sizeof(PageHeader) +
		rid.slot_id * sizeof(Slot));
	if (slot->length == 0) {
		buffer_pool_manager_.UnpinPage(rid.page_id, false);
		return false;
	}

	const uint16_t tuple_size = static_cast<uint16_t>(size);
	if (tuple_size <= slot->length) {
		std::memcpy(page->data + slot->offset, data, tuple_size);
		slot->length = tuple_size;
		buffer_pool_manager_.UnpinPage(rid.page_id, true);
		return true;
	}

	uint16_t available_space = static_cast<uint16_t>((header->free_space_offset + 1) -
		(sizeof(PageHeader) + header->slot_count * sizeof(Slot)));
	if (tuple_size > available_space) {
		buffer_pool_manager_.UnpinPage(rid.page_id, false);
		return false;
	}

	uint16_t new_offset = static_cast<uint16_t>(header->free_space_offset - tuple_size + 1);
	std::memcpy(page->data + new_offset, data, tuple_size);
	slot->offset = new_offset;
	slot->length = tuple_size;
	header->free_space_offset = static_cast<uint16_t>(new_offset - 1);
	buffer_pool_manager_.UnpinPage(rid.page_id, true);
	return true;
}


uint16_t RecordManager::GetSlotCount(page_id_t page_id) {
	Page* page = buffer_pool_manager_.FetchPage(page_id);
	if (!page) return 0;
	PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
	uint16_t count = header->slot_count;
	buffer_pool_manager_.UnpinPage(page_id, false);
	return count;
}