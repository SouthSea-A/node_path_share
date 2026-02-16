# node_path_share 路径共享系统
## 概述
### node_path_share 是一个用于高效存储和管理多条路径（节点序列）的 C++ 库。其核心设计目标是通过共享路径中的公共前缀来减少内存占用，并支持对路径的常见操作（增、删、改、查、分支、交换片段等）。系统内部使用前驱链表结构存储节点之间的连接关系，每个节点（ahead）记录上一个节点的存储位置、节点编号以及引用计数，从而实现多条路径对相同前缀的共享。

## 数据类型与常量
### 基础类型别名
|别名|实际类型|描述|
|-|-|-|
|data_type_base_unsigned|uint64_t|无符号基础类型|
data_type_base_signed|int64_t|有符号基础类型|
node_index|data_type_base_unsigned|节点编号（用户赋予的标识）|
node_pos|data_type_base_unsigned|节点在内部存储 node_set 中的位置|
node_pos_get|data_type_base_unsigned|节点在路径中的位置索引（从0开始）|
node_ref|data_type_base_unsigned|节点的引用计数|
path_index|data_type_base_unsigned|路径编号|
path_pos|data_type_base_unsigned|路径信息在内部 path_set 中的位置|
path_len|data_type_base_unsigned|路径长度（节点个数）|
### 常量
|常量名|值|描述|
|-|-|-|
|NODE_INVALID_INDEX|0xFFFFFFFFFFFFFFFF|无效的节点编号|
|NODE_INVALID_POS|0xFFFFFFFFFFFFFFFF|无效的节点存储位置|
|NODE_INVALID_POS_GET|0xFFFFFFFFFFFFFFFF|无效的路径节点位置|
|NODE_INVALID_REF|0xFFFFFFFFFFFFFFFF|无效的引用计数|
|NODE_ROOT_POS|0xFFFFFFFFFFFFFFFE|根节点位置（前驱链表的结束标记）|
|PATH_INVALID_INDEX|0xFFFFFFFFFFFFFFFF|无效的路径编号|
|PATH_INVALID_POS|0xFFFFFFFFFFFFFFFF|无效的路径信息存储位置|
|PATH_INVALID_LEN|0xFFFFFFFFFFFFFFFF|无效的路径长度|
### 辅助结构体
### ahead
前驱节点结构，用于构建节点之间的连接。
```
struct ahead
{
    node_pos last;      // 上一个前驱的存储位置
    node_index index;   // 当前节点的编号
    node_ref ref;       // 当前节点的引用计数
    // 构造函数、拷贝构造函数、有效性检查函数...
};
```
### path_info_base
路径基本信息，记录路径的入口和中点位置。
```
struct path_info_base
{
    node_pos last_pos;  // 最后一个前驱的存储位置（路径入口）
    node_pos mid_pos;   // 中间位置节点所在前驱的位置（用于优化访问）
    path_len len;       // 路径长度
    // 构造函数、拷贝构造函数、有效性检查...
};
```
### path_info
路径信息，包含路径编号和基本信息。
```
struct path_info
{
    path_index index;   // 路径编号
    path_info_base info;// 基本信息
    // 构造函数、有效性检查...
};
```
### path
完整的路径表示（用于导出或导入路径）。
```
struct path
{
    path_index index;               // 路径编号
    std::vector<node_index> nodes;  // 路径包含的所有节点（顺序存储）
    // 构造函数、移动构造、赋值运算符...
};
```
### path_part
用于表示路径中的一个连续片段。
```
struct path_part
{
    path_index index;      // 所属路径的编号
    node_pos_get pos_start; // 片段起点（包含）
    node_pos_get pos_end;   // 片段终点（包含）
    // 构造函数...
};
```
### path_unify_construct
用于路径整合时的临时结构，表示一个前驱节点与其上一个节点的关系。
```
struct path_unify_construct
{
    node_pos last;      // 上一个前驱的存储位置
    node_index current; // 当前节点的编号
    // 构造函数、相等比较...
};
```
### path_unify_hash
为 path_unify_construct 提供的哈希函数对象，用于 std::unordered_map。

### path_state_statistic
系统状态统计信息。
```
struct path_state_statistic
{
    size_t quantity_path;               // 路径数量
    size_t quantity_bytes_memory_alloc; // 已分配的总内存字节数
    size_t quantity_bytes_memory_used;  // 实际有效使用的内存字节数
    float rate_memory_used;             // 内存使用率（有效/分配）
    // 构造函数...
};
```
### 模板类 node<node_data>
用于封装节点数据，与路径系统配合使用（但路径系统本身不强制使用此类，仅作为建议的数据包装）。
#### 公有成员函数
|函数|描述|
|-|-|
|node()|默认构造，节点编号为无效值，数据默认构造。|
|node(const node_data& data_)|用数据构造，节点编号为无效值。|
|node(node_data&& data_)|用数据移动构造，节点编号为无效值。|
|node(node_index index_, const node_data& data_)|用编号和数据构造。|
|node(node_index index_, node_data&& data_)|用编号和数据移动构造。|
|~node()|析构函数。|
|node_index get_index() const|获取节点编号。|
|node_data& get_data_ref()|获取数据引用。|
|const node_data& get_const_data_ref() const|获取数据常引用。|
|bool get_validity_node_index() const|判断节点编号是否有效。|
|bool set_index(node_index index)|设置节点编号，若 index 无效则返回 false。|
|void set_data(const node_data& data)|设置数据（拷贝）。|
|void set_data(node_data&& data)|设置数据（移动）。|

### 核心类 path_share
path_share 是路径共享系统的主类，负责所有路径的存储、共享和操作。

#### 构造与析构
|函数|描述|
|-|-|
|path_share()|默认构造，创建一个空系统。|
|path_share(const path_share&)|拷贝构造，复制整个系统状态。|
|path_share(path_share&&) noexcept|移动构造。|
|~path_share()|析构函数，释放所有资源。|
#### 成员函数
##### 系统整合与状态
|函数|描述|
|-|-|
|bool unify()|整合全部路径，使相同的前缀节点被共享，同时整理内存碎片。通常在大量路径修改后调用，以优化内存使用。返回 true 表示整合成功。|
|path_state_statistic get_stastic() const|获取当前系统的状态统计信息，包括路径数量、内存分配、使用情况等。|
|void clear()|清空整个系统，删除所有路径和节点，释放内存。|
|size_t get_path_quantity() const|获取系统中当前存在的路径数量。|
##### 路径查询
|函数|描述|
|-|-|
|node_index get_path_node_index(path_index path, node_pos_get pos) const|获取指定路径中 pos 位置处的节点编号。若路径无效或位置越界，返回 NODE_INVALID_INDEX。|
|bool get_path_node_index_mul(path_index path, std::vector<node_pos_get> pos_mul, std::vector<node_pos>& save) const|批量获取路径中多个位置对应的内部存储位置（node_pos，即前驱地址）。pos_mul 需按升序排列。结果按输入顺序保存在 save 中。若失败返回 false（如路径无效、存在越界位置等）。此函数主要用于内部优化，普通用户可直接使用 get_path_node_index 获取节点编号。|
|bool get_path(path_index path_index, path& save) const|获取指定路径的完整节点序列，保存到 save 中。返回 true 表示成功，否则 false（路径不存在）。|
|bool get_path_all(std::vector<path>& save) const|获取系统中所有路径的完整节点序列，保存到 save 中。返回 true 表示成功，若中途出错返回 false。|
##### 路径创建与分支
|函数|描述|
|-|-|
|path_index create_path(node_index index)|创建一条只包含一个节点的新路径，节点编号为 index。返回新路径的编号，若创建失败返回 PATH_INVALID_INDEX。|
|path_index create_branch(path_index path, node_pos_get make_pos, node_index index)|基于已有路径 path，在位置 make_pos 处之后插入一个新节点 index，形成一条新路径（原路径不变）。新路径的长度为 make_pos + 2（即从开头到新节点）。返回新路径编号，失败返回 PATH_INVALID_INDEX。|
##### 路径修改
|函数|描述|
|-|-|
|bool insert_path_node(path_index path, node_pos_get pos, node_index index)|在路径 path 的 pos 位置之前插入一个新节点 index（插入后该节点成为新位置 pos 处的节点）。操作会尽量共享已有前缀，必要时复制节点。返回 true 表示成功。|
|bool add_path_node(path_index path, std::vector<node_index> add_node)|在路径 path 的末尾批量添加一系列节点。返回 true 表示成功。|
|bool add_path_node_single(path_index path, node_index add_node)|在路径 path 的末尾添加单个节点。|
|bool change_path_node(path_index path, node_pos_get pos, node_index index)|修改路径 path 中位置 pos 处的节点编号为 index。返回 true 表示成功。|
|bool del_path_node(path_index path, std::vector<node_pos_get> del_node)|批量删除路径 path 中指定位置的节点（位置可重复，会自动去重）。删除后剩余节点保持原有顺序。若删除后路径长度为0，该路径会被自动移除。返回 true 表示成功。|
|bool del_path_node_single(path_index path, node_pos_get del_node)|删除路径 path 中指定位置的单个节点。|
|bool del_path(path_index path)|删除整个路径 path，同时释放不再被共享的节点。返回 true 表示成功。|
##### 路径片段交换
|函数|描述|
|-|-|
|bool exchange_path_part(const path_part& path_one, const path_part& path_sec)|交换两个路径片段。两个片段可以属于同一条路径（需保证不重叠），也可以属于不同路径。片段由 path_part 结构指定（包含路径编号、起点和终点）。交换后两条路径的长度可能发生变化。返回 true 表示成功。|
##### 运算符重载
为方便使用，path_share 提供了几个运算符重载：
|运算符|等价函数|描述|
|-|-|-|
|node_index operator()(path_index path, node_pos_get pos)|get_path_node_index|获取路径指定位置的节点编号。|
|bool operator()(path_index path, node_pos_get pos, node_index index)|change_path_node|修改路径指定位置的节点编号。|
|bool operator()(path_index path_index, path& save) const|get_path|获取路径的完整节点序列。|
|path_share& operator=(const path_share& other)|拷贝赋值	深拷贝另一个系统。|
|path_share& operator=(path_share&& other) noexcept|移动赋值|移动另一个系统的资源。|

### 注意事项
- 路径位置索引：所有 node_pos_get 类型的参数表示路径中的位置，从0开始（0为第一个节点）。必须小于路径长度，否则操作会失败。

- 节点编号有效性：创建或修改节点时，传入的 node_index 不能为 NODE_INVALID_INDEX。

- 引用计数：系统内部自动维护引用计数，当某个节点的引用降为0时，该节点会被删除并回收空间。用户无需手动管理。

- 内存碎片：频繁修改路径可能产生内部碎片，可通过调用 unify() 进行整理，使共享最大化并压缩存储。

- 线程安全：path_share 本身不是线程安全的，多线程环境下需要外部加锁。

- 批量操作优化：如 get_path_node_index_mul 要求输入位置升序，否则行为未定义。建议使用排序后的容器。

版本与依赖
需要 C++11 或更高版本支持（使用了移动语义、lambda、nullptr 等特性）。

无外部依赖，仅使用标准库。
