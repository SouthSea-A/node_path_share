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
完整的路径表示（用于导出路径）。
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
|node()|默认构造，节点编号为无效值，数据默认构造|
|node(const node_data& data_)|用数据构造，节点编号为无效值|
|node(node_data&& data_)|用数据移动构造，节点编号为无效值|
|node(node_index index_, const node_data& data_)|用编号和数据构造|
|node(node_index index_, node_data&& data_)|用编号和数据移动构造|
|~node()|析构函数。|
|node_index get_index() const|获取节点编号|
|node_data& get_data_ref()|获取数据引用|
|const node_data& get_const_data_ref() const|获取数据常引用|
|bool get_validity_node_index() const|判断节点编号是否有效|
|bool set_index(node_index index)|设置节点编号，若 index 无效则返回 false。|
|void set_data(const node_data& data)|设置数据（拷贝）|
|void set_data(node_data&& data)|设置数据（移动）|
### 核心类 path_share
path_share 是路径共享系统的主类，负责所有路径的存储、共享和操作。
#### 构造与析构
|函数|描述|
|-|-|
|path_share()|默认构造，创建一个空系统|
|path_share(const path_share&)|拷贝构造，复制整个系统状态|
|path_share(path_share&&) noexcept|移动构造|
|~path_share()|析构函数，释放所有资源|
#### 成员函数
##### 系统整合与状态
|函数|描述|
|-|-|
|bool unify()|整合全部路径，使相同的前缀节点被共享，同时整理内存碎片。通常在大量路径修改后调用，以优化内存使用。返回 true 表示整合成功|
|path_state_statistic get_stastic() const|获取当前系统的状态统计信息，包括路径数量、内存分配、使用情况等|
|void clear()|清空整个系统，删除所有路径和节点，释放内存|
|size_t get_path_quantity() const|获取系统中当前存在的路径数量|
##### 路径查询
|函数|描述|
|-|-|
|node_index get_path_node_index(path_index path, node_pos_get pos) const|获取指定路径中 pos 位置处的节点编号。若路径无效或位置越界，返回 NODE_INVALID_INDEX|
|bool get_path_node_index_mul(path_index path, std::vector<node_pos_get> pos_mul, std::vector<node_pos>& save) const|批量获取路径中多个位置对应的内部存储位置（node_pos，即前驱地址）。pos_mul 需按升序排列。结果按输入顺序保存在 save 中。若失败返回 false（如路径无效、存在越界位置等）。此函数主要用于内部优化，普通用户可直接使用 get_path_node_index 获取节点编号|
|bool get_path(path_index path_index, path& save) const|获取指定路径的完整节点序列，保存到 save 中。返回 true 表示成功，否则 false（路径不存在）|
|bool get_path_all(std::vector<path>& save) const|获取系统中所有路径的完整节点序列，保存到 save 中。返回 true 表示成功，若中途出错返回 false|
##### 路径创建与分支
|函数|描述|
|-|-|
|path_index create_path(node_index index)|创建一条只包含一个节点的新路径，节点编号为 index。返回新路径的编号，若创建失败返回 PATH_INVALID_INDEX|
|path_index create_branch(path_index path, node_pos_get make_pos, node_index index)|基于已有路径 path，在位置 make_pos 处之后插入一个新节点 index，形成一条新路径（原路径不变）。新路径的长度为 make_pos + 2（即从开头到新节点）。返回新路径编号，失败返回 PATH_INVALID_INDEX|
##### 路径修改
|函数|描述|
|-|-|
|bool insert_path_node(path_index path, node_pos_get pos, node_index index)|在路径 path 的 pos 位置之前插入一个新节点 index（插入后该节点成为新位置 pos 处的节点）。操作会尽量共享已有前缀，必要时复制节点。返回 true 表示成功|
|bool add_path_node(path_index path, std::vector<node_index> add_node)|在路径 path 的末尾批量添加一系列节点。返回 true 表示成功|
|bool add_path_node_single(path_index path, node_index add_node)|在路径 path 的末尾添加单个节点|
|bool change_path_node(path_index path, node_pos_get pos, node_index index)|修改路径 path 中位置 pos 处的节点编号为 index。返回 true 表示成功|
|bool del_path_node(path_index path, std::vector<node_pos_get> del_node)|批量删除路径 path 中指定位置的节点（位置可重复，会自动去重）。删除后剩余节点保持原有顺序。若删除后路径长度为0，该路径会被自动移除。返回 true 表示成功|
|bool del_path_node_single(path_index path, node_pos_get del_node)|删除路径 path 中指定位置的单个节点|
|bool del_path(path_index path)|删除整个路径 path，同时释放不再被共享的节点。返回 true 表示成功|
##### 路径片段交换
|函数|描述|
|-|-|
|bool exchange_path_part(const path_part& path_one, const path_part& path_sec)|交换两个路径片段。两个片段可以属于同一条路径（需保证不重叠），也可以属于不同路径。片段由 path_part 结构指定（包含路径编号、起点和终点）。交换后两条路径的长度可能发生变化。返回 true 表示成功|
##### 运算符重载
为方便使用，path_share 提供了几个运算符重载：
|运算符|等价函数|描述|
|-|-|-|
|node_index operator()(path_index path, node_pos_get pos)|get_path_node_index|获取路径指定位置的节点编号|
|bool operator()(path_index path, node_pos_get pos, node_index index)|change_path_node|修改路径指定位置的节点编号|
|bool operator()(path_index path_index, path& save) const|get_path|获取路径的完整节点序列|
|path_share& operator=(const path_share& other)|拷贝赋值	深拷贝另一个系统|
|path_share& operator=(path_share&& other) noexcept|移动赋值|移动另一个系统的资源|
### 注意事项
- 路径位置索引：所有 node_pos_get 类型的参数表示路径中的位置，从0开始（0为第一个节点）。必须小于路径长度，否则操作会失败。
- 节点编号有效性：创建或修改节点时，传入的 node_index 不能为 NODE_INVALID_INDEX。
- 引用计数：系统内部自动维护引用计数，当某个节点的引用降为0时，该节点会被删除并回收空间。用户无需手动管理。
- 内存碎片：频繁修改路径可能产生内部碎片，可通过调用 unify() 进行整理，使共享最大化并压缩存储。
- 线程安全：path_share 本身不是线程安全的，多线程环境下需要外部加锁。
- 批量操作优化：如 get_path_node_index_mul 要求输入位置升序，否则行为未定义。建议使用排序后的容器。
### node_path_share 系统特性
#### 1. 核心设计理念
#### 1.1 前驱链表结构
系统采用反向链接的存储方式，每个节点（ahead）只记录其上一个节点的位置，而不是下一个节点。这种设计使得：
- 多条路径可以共享相同的后缀（实际上是前缀，因为是反向的）
- 路径的入口是最后一个节点，遍历时需要反向读取
- 插入和删除操作只需要修改局部链接，无需移动大量数据
#### 1.2 引用计数共享机制
每个节点都维护一个引用计数（ref），记录有多少条路径正在使用该节点：
- 引用计数 > 1：节点被多条路径共享，修改时需要先复制（写时复制）
- 引用计数 = 1：节点只被一条路径使用，可以直接修改
- 引用计数 = 0：节点被自动回收，空间加入空闲列表
#### 1.3 中点位置优化
每条路径都记录了中间节点的存储位置（mid_pos），用于加速随机访问：
- 访问路径中的任意位置时，可以从头部或尾部向中间查找，减少遍历步数
- 当路径长度变化时，中点位置会自动更新
- 批量位置查询时，系统会根据中点位置选择最优遍历方向
### 2. 内存管理特性
#### 2.1 空闲列表复用
系统维护两个空闲列表：
- node_set_free：记录已被删除的节点存储位置，新节点优先复用这些位置
- path_index_free：记录已被删除的路径编号，新路径优先复用这些编号
这种设计避免了频繁的内存分配和释放，提高了性能。
#### 2.2 路径信息紧凑存储
路径信息（path_info）按编号升序连续存储在 path_set 中，不留空洞：
- 通过二分查找快速定位路径信息
- 删除路径后，后续元素向前移动，保持紧凑
- 路径编号回收机制确保编号不会无限增长
#### 2.3 内存状态监控
系统提供详细的内存统计信息（path_state_statistic）：
- 路径数量
- 已分配内存总量
- 有效使用内存量
- 内存使用率
- 用户可以监控系统的内存效率，适时调用 unify() 进行整理。
### 3. 路径操作特性
#### 3.1 写时复制（Copy-on-Write）
当需要修改一个被共享的节点时，系统会自动：
- 复制该节点及其受影响的后继节点
- 减少原节点的引用计数
- 更新路径的入口和中点位置
- 保持其他路径不变
- 这保证了共享路径之间的隔离性，修改一条路径不会影响其他路径。
#### 3.2 批量操作优化
系统针对批量操作进行了专门优化：
- 批量查询：get_path_node_index_mul 支持一次性查询多个位置，内部会根据位置分布选择最优遍历路径，减少重复遍历
- 批量添加：add_path_node 可以一次性在路径末尾添加多个节点
- 批量删除：del_path_node 支持删除多个位置，自动处理去重和重连
#### 3.3 片段交换
exchange_path_part 支持复杂的路径片段交换操作：
- 同一路径内交换两个片段（要求不重叠）
- 不同路径之间交换片段
- 自动处理长度变化和引用计数调整
- 交换后自动更新中点位置
#### 3.4 分支创建
create_branch 允许基于现有路径创建新分支：
- 在指定位置后插入新节点形成新路径
- 原路径完全保持不变
- 新路径与原路径共享相同的前缀
- 自动处理引用计数，无需手动管理
### 4. 路径整合特性
#### 4.1 自动共享检测
unify_node_set（通过 unify() 调用）能够：
- 从所有路径中提取完整的节点序列
- 按层级（从根到叶）统计相同的前缀组合
- 为每层相同的前缀创建共享节点
- 更新所有路径的入口和中点位置
- 回收不再使用的孤立节点
#### 4.2 层级整合算法
整合过程采用广度优先的方式：
- 第0层：所有路径的第一个节点，相同的组合共享
- 第1层：第二个节点，考虑与前一个节点的关系
- 以此类推，直到所有路径处理完毕
- 这种算法确保了最大程度的共享。
#### 4.3 碎片整理
unify() 不仅可以最大化共享，还能：
- 消除删除操作产生的空洞
- 压缩节点存储，提高内存利用率
- 优化路径的物理布局，提高访问速度
### 5. 性能特性
#### 5.1 时间复杂度
|操作分类|具体接口|时间复杂度|说明|
|-|-|-|-|
|创建类|create_path|均摊 O(1)|仅新建 1 个前驱节点 + 1 个路径元数据，vector 尾部插入为均摊 O(1)|
||create_branch|O(L)|L为分叉位置到路径末尾的长度，需自增该段节点的引用计数|
|插入类|insert_path_node|O(L)|L为插入位置到末尾的长度，需先执行写时复制（COW）复制该段共享节点|
||add_path_node（批量末尾）|O(L/2+k)|k为新增节点数，L/2为计算新中点的最坏开销|
||add_path_node_single|O(L/2)|单节点末尾插入，仅需计算新中点（若需）|
|修改类|change_path_node|O(L)|L为修改位置到末尾的长度，需先执行 COW 复制该段共享节点|
|删除类|del_path_node_single|O(L)|L为删除位置到末尾的长度，需 COW 复制并重连链表|
||del_path_node（批量）|O(mlogm+L)|m为删除位置数，需先排序去重；L为最小删除位置到末尾的长度|
||del_path|O(L)|需从末尾遍历到起点，展开完整路径|
|查询类|get_path_node_index|	O(L/2)|中点优化，最坏仅需遍历半条路径|
||get_path_node_index_mul|O(L/2+m)|批量查询，一次遍历完成所有请求，m为查询位置数|
||get_path|O(L)|需从末尾遍历到起点，展开完整路径|
||get_path_all|O(∑L_i)|需展开所有路径，∑L_i为所有路径总长度|
|高级操作|exchange_path_part|O(L_1+L_2)|L_1,L_2为两条路径交换起点到末尾的长度，需 COW 复制并重连|
||unify（碎片整理 / 最大化共享）|O(∑L_i)|需先展开所有路径，再按层级重建节点池，∑L_i为所有路径总节点数|
#### 5.2 空间效率
- 每个节点额外开销：3个64位整数（last, index, ref）
- 每条路径额外开销：3个64位整数（last_pos, mid_pos, len）+ 路径编号
- 通过共享，存储空间随路径相似度提高而降低
- 空闲列表机制避免内存碎片
#### 5.3 缓存友好性
- 路径信息连续存储，利于缓存预取
- 节点存储虽然有空洞，但 unify() 后也是连续的
- 批量操作时按顺序访问，提高缓存命中率
### 版本与依赖
- 需要 C++11 或更高版本支持
- 无外部依赖，仅使用标准库。
