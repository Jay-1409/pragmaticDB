# StableDB — System Architecture

## How to use in Excalidraw
Go to **Insert → Mermaid**, paste the code block below, and click **Insert**.

---

## Full Architecture Diagram

```mermaid
flowchart TD
    %% ── Styles ──────────────────────────────────────────────────────────────
    classDef ds       fill:#16213e,stroke:#e94560,stroke-width:2px,color:#eee
    classDef storage  fill:#0f3460,stroke:#ff6b6b,stroke-width:2px,color:#eee
    classDef record   fill:#1a1a2e,stroke:#fca311,stroke-width:2px,color:#eee
    classDef typemod  fill:#1b2838,stroke:#ffe66d,stroke-width:2px,color:#eee
    classDef catalog  fill:#0d2137,stroke:#4ecdc4,stroke-width:2px,color:#eee
    classDef future   fill:#111,stroke:#555,stroke-width:1px,color:#777,stroke-dasharray:6

    %% ── Layer 0 : Data Structures (ds/) ──────────────────────────────────
    subgraph DS["📦  Data Structures  (include/ds/)"]
        direction LR
        PAGE["Page\n──────\npage_id: page_id_t\npin_count: int\nis_dirty: bool\ndata: char[4096]"]:::ds
        SLOT["Slot\n──────\noffset: uint16_t\nlength: uint16_t"]:::ds
        PHDR["PageHeader\n──────\npage_id: page_id_t\nfree_space_offset: uint16_t\nslot_count: uint16_t"]:::ds
        RID["RecordId\n──────\npage_id: page_id_t\nslot_id: uint16_t"]:::ds
        PAGE -- embedded in --> PHDR
        PAGE -- contains array of --> SLOT
        RID -- references --> PAGE
        RID -- references --> SLOT
    end

    %% ── Layer 1 : Storage Engine (manager/) ─────────────────────────────
    subgraph STORAGE["💾  Storage Engine  (src/manager/)"]
        direction TB
        DM["DiskManager\n──────\nWritePage(id, data)\nReadPage(id, data)"]:::storage

        BPM["BufferPoolManager\n──────\npool_size_: size_t\npages_: vector&lt;Page&gt;\npage_table_: unordered_map\nfree_list_: deque\nlru_list_: list  ← LRU eviction\n──────\nNewPage() → Page*\nFetchPage(id) → Page*\nUnpinPage(id, dirty)\nFlushPage(id)\nFlushAllPages()\nDeletePage(id)"]:::storage

        PDM["PageDataManager\n──────\nInsertTuple(page, data, size, &slot_id)\nGetTuple(page, slot_id, &data, &size)\nDeleteTuple(page, slot_id)\nCompactOnePage(page)\n──────\nSlot-directory layout:\n  [PageHeader | Slots→] [←Tuples]"]:::storage

        BPM -->|"holds pointer to"| DM
        BPM -->|"exposes Page*"| PDM
    end

    %% ── Layer 2 : Record Manager ─────────────────────────────────────────
    subgraph RECMGR["📋  Record Manager  (src/manager/)"]
        RM["RecordManager\n──────\nInsert(data, size) → RecordId\nGet(rid, &data, &size)\nUpdate(rid, data, size)\nDelete(rid)\n──────\nOwns: DiskManager\n       BufferPoolManager\n       PageDataManager\nTracks: page_ids_: vector"]:::record
    end

    STORAGE -->|"used internally by"| RM

    %% ── Layer 3 : Type System (type/) ────────────────────────────────────
    subgraph TYPESYS["🔢  Type System  (include/type/ & src/type/)"]
        direction TB
        TYPEID["TypeId  (enum)\n──────\nINVALID = 0\nBOOLEAN   ← 1 byte\nINTEGER   ← 4 bytes"]:::typemod

        FACTORY["DsFactory\n──────\nget_structure(TypeId) → std::any\n(Factory pattern — allocates\ncorrect DS per type)"]:::typemod

        VAL["Value\n──────\ntype_id_: TypeId\ndata_: std::any\n──────\nGet&lt;T&gt;() → T\nSet&lt;T&gt;(val)\nSerializeToChar(char*)\nDeserializeFromChar(char*, TypeId)"]:::typemod

        TUP["Tuple\n──────\ndata_: vector&lt;char&gt;\n──────\nTuple(values, schema)  ← Serialize all Values\nTuple(raw_data, size)  ← Reconstruct from disk\nGetValue(schema, col_idx) → Value\nGetData() → const char*\nGetLength() → uint32_t"]:::typemod

        TYPEID -->|"used by"| FACTORY
        TYPEID -->|"used by"| VAL
        FACTORY -.->|"called by (intended)"| VAL
        VAL -->|"serialized into"| TUP
    end

    %% ── Layer 4 : Catalog (catalog/) ─────────────────────────────────────
    subgraph CATMOD["📚  Catalog Module  (include/catalog/ & src/catalog/)"]
        direction TB
        COL["Column\n──────\ncolumn_name_: string\ntype_id_: TypeId\nlength_: uint32_t  ← auto (4 / 1)\n──────\nGetName() / GetType() / GetLength()"]:::catalog

        SCH["Schema\n──────\ncolumns_: vector&lt;Column&gt;\noffsets_: vector&lt;uint32_t&gt;\nlength_: uint32_t\n──────\nGetColOffset(idx) → uint32_t  ← O(1)\nGetColumn(idx) → Column&\nGetLength() → uint32_t\nGetColumnCount() → uint32_t"]:::catalog

        TINFO["TableInfo  (struct)\n──────\nname_: string\noid_: table_oid_t\nschema_: Schema\ntable_: unique_ptr&lt;TableManager&gt;"]:::catalog

        CAT["Catalog\n──────\ntables_: unordered_map&lt;oid, TableInfo&gt;\ntable_names_: unordered_map&lt;name, oid&gt;\nnext_table_oid_: uint32_t\n──────\nCreateTable(name, schema) → TableInfo*\nGetTable(name) → TableInfo*  throws if missing\nGetTable(oid)  → TableInfo*  throws if missing"]:::catalog

        COL -->|"composed into"| SCH
        SCH -->|"stored in"| TINFO
        TINFO -->|"registered in"| CAT
    end

    %% ── Layer 5 : Table Manager ──────────────────────────────────────────
    subgraph TMGR["🗄️  Table Manager  (src/manager/)"]
        TM["TableManager\n──────\nrecord_manager_: RecordManager\n──────\nInsertTuple(tuple) → RecordId\nGetTuple(rid, schema) → Tuple"]:::record
    end

    TINFO -->|"owns"| TM
    TM -->|"delegates to"| RM

    %% ── Future Layers ────────────────────────────────────────────────────
    subgraph FUTURE["🚧  Planned Modules  (src/network & src/query — empty)"]
        direction LR
        ITER["TableIterator\n(not yet built)"]:::future
        NET["NetworkLayer\n(not yet built)"]:::future
        QRY["QueryEngine / SQL Parser\n(not yet built)"]:::future
    end

    %% ── Cross-layer Data Flow ────────────────────────────────────────────
    SCH -.->|"provides byte offsets for"| TUP
    TUP -->|"GetData() → raw char*"| TM
    TM -->|"InsertTuple / GetTuple"| TUP

    CAT -.->|"entry point for"| FUTURE
```

---

## Key Data Flow: Write Path
```
User Code
  → Catalog.CreateTable("users", schema)         # register table
  → TableManager.InsertTuple(tuple)              # pass logical row
    → Tuple.GetData() / GetLength()              # extract raw bytes
      → RecordManager.Insert(char*, size)        # hand off to storage
        → PageDataManager.InsertTuple(Page*, …)  # write to page slot
          → BufferPoolManager.NewPage()          # allocate/fetch page frame
            → DiskManager.WritePage()            # flush dirty page to disk
```

## Key Data Flow: Read Path
```
User Code
  → Catalog.GetTable("users")                    # lookup by name (O(1) hash)
  → TableManager.GetTuple(RecordId, schema)      # request a row by location
    → RecordManager.Get(rid, buffer, &size)       # fetch raw bytes
      → PageDataManager.GetTuple(Page*, slot_id) # read from slot directory
        → BufferPoolManager.FetchPage(page_id)   # hit cache or load from disk
    → Tuple(buffer, size)                        # reconstruct from raw bytes
  → Tuple.GetValue(schema, col_idx)              # deserialize one column
    → Value.DeserializeFromChar(src, TypeId)     # produce typed Value
```
