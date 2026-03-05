# node_path_share

<p>一个高性能、内存高效的路径共享系统，通过反向前驱链表+引用计数机制最大化减少重复路径的内存占用，适配导航路径、文件系统、版本控制、图算法等多场景。</p>

<h2>✨ 核心特性</h2>

<ul>
  <li><strong>极致内存效率</strong>：全局节点段共享，高重叠路径场景下内存占用可降至传统方案的1/10~1/100</li>
  <li><strong>高性能查询优化</strong>：中点缓存机制，将路径遍历复杂度从O(N)优化至O(N/2)</li>
  <li><strong>精细化内存复用</strong>：栈式空闲列表机制，复用删除产生的内存空洞，减少内存碎片</li>
  <li><strong>全链路安全校验</strong>：完善的合法性校验，避免无效内存访问、越界操作等未定义行为</li>
  <li><strong>灵活可扩展</strong>：模板化设计，支持任意自定义节点数据类型，无业务侵入</li>
  <li><strong>全场景接口覆盖</strong>：提供路径创建、分支、插入、删除、修改、交换、整合全生命周期操作</li>
</ul>

<h2>🚀 快速开始</h2>

<pre><code class="language-cpp">// 1. 引入头文件
#include "node_path_share.h"

// 2. 使用命名空间
using namespace node_path;

// 3. 初始化路径共享系统
path_share ps;

// 4. 创建单节点起始路径
path_index main_path = ps.create_path(1001);

// 5. 批量追加节点
ps.add_path_node(main_path, {1002, 1003, 1004, 1005});

// 6. 基于主路径创建分支
path_index branch_path = ps.create_branch(main_path, 2, 2001);

// 7. 获取完整路径
path result;
ps.get_path(branch_path, result);

// 8. 快速查询/修改节点
node_index node = ps(main_path, 3);  // 获取路径指定位置节点
ps(main_path, 2, 1006);             // 修改路径指定位置节点
</code></pre>

<h2>📦 核心数据结构</h2>

<h3>基础类型定义</h3>

<pre><code class="language-cpp">namespace node_path {
    // 基础数据类型
    using data_type_base_unsigned = uint64_t;
    using data_type_base_signed = int64_t;
    
    // 节点相关类型
    using node_index = data_type_base_unsigned;      // 节点唯一编号
    using node_pos = data_type_base_unsigned;        // 节点物理存储位置
    using node_pos_get = data_type_base_unsigned;    // 节点在路径中的逻辑位置
    using node_ref = data_type_base_unsigned;        // 节点共享引用计数
    
    // 路径相关类型
    using path_index = data_type_base_unsigned;      // 路径唯一编号
    using path_pos = data_type_base_unsigned;        // 路径信息物理存储位置
    using path_len = data_type_base_unsigned;        // 路径长度
}
</code></pre>

<h3>全局常量定义</h3>

<pre><code class="language-cpp">// 无效值常量
constexpr node_index NODE_INVALID_INDEX = 0xFFFFFFFFFFFFFFFF;
constexpr node_pos NODE_INVALID_POS = 0xFFFFFFFFFFFFFFFF;
constexpr path_index PATH_INVALID_INDEX = 0xFFFFFFFFFFFFFFFF;
constexpr path_len PATH_INVALID_LEN = 0xFFFFFFFFFFFFFFFF;

// 根节点位置
constexpr node_pos NODE_ROOT_POS = 0xFFFFFFFFFFFFFFFE;
</code></pre>

<h2>📖 完整接口文档</h2>

<h3>1. 路径创建与分支</h3>

<table>
  <thead>
    <tr>
      <th>函数接口</th>
      <th>功能说明</th>
      <th>参数说明</th>
      <th>返回值</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>path_index create_path(node_index index)</code></td>
      <td>创建单节点起始路径</td>
      <td><code>index</code>：起始节点编号</td>
      <td>新路径编号，创建失败返回 <code>PATH_INVALID_INDEX</code></td>
    </tr>
    <tr>
      <td><code>path_index create_branch(path_index path, node_pos_get make_pos, node_index index)</code></td>
      <td>基于已有路径的指定位置创建分支路径</td>
      <td><code>path</code>：原路径编号<br><code>make_pos</code>：分支节点的逻辑位置<br><code>index</code>：分支新增的节点编号</td>
      <td>新分支路径编号，创建失败返回 <code>PATH_INVALID_INDEX</code></td>
    </tr>
  </tbody>
</table>

<h3>2. 路径查询接口</h3>

<table>
  <thead>
    <tr>
      <th>函数接口</th>
      <th>功能说明</th>
      <th>参数说明</th>
      <th>返回值</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>bool get_path(path_index path_index, path& save) const</code></td>
      <td>获取指定路径的完整节点列表</td>
      <td><code>path_index</code>：目标路径编号<br><code>save</code>：输出参数，存储完整路径</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>bool get_path_all(std::vector<path>& save) const</code></td>
      <td>获取系统中所有路径的完整数据</td>
      <td><code>save</code>：输出参数，存储所有路径</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>node_index get_path_node_index(path_index path, node_pos_get pos) const noexcept</code></td>
      <td>获取路径指定位置的节点编号</td>
      <td><code>path</code>：目标路径编号<br><code>pos</code>：节点在路径中的逻辑位置</td>
      <td>节点编号，失败返回 <code>NODE_INVALID_INDEX</code></td>
    </tr>
    <tr>
      <td><code>bool get_path_node_index_mul(path_index path, const std::vector<node_pos_get>& pos_mul, std::vector<node_index>& save) const</code></td>
      <td>批量获取路径多个指定位置的节点编号</td>
      <td><code>path</code>：目标路径编号<br><code>pos_mul</code>：待查询的逻辑位置列表（需升序）<br><code>save</code>：输出参数，存储查询结果</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>path_len get_path_len(path_index path) const noexcept</code></td>
      <td>获取指定路径的长度</td>
      <td><code>path</code>：目标路径编号</td>
      <td>路径长度，失败返回 <code>PATH_INVALID_LEN</code></td>
    </tr>
    <tr>
      <td><code>size_t get_path_quantity() const noexcept</code></td>
      <td>获取系统中当前的路径总数</td>
      <td>无</td>
      <td>路径总数</td>
    </tr>
  </tbody>
</table>

<h3>3. 路径修改接口</h3>

<table>
  <thead>
    <tr>
      <th>函数接口</th>
      <th>功能说明</th>
      <th>参数说明</th>
      <th>返回值</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>bool add_path_node(path_index path, const std::vector<node_index>& add_node)</code></td>
      <td>批量追加节点到路径末尾</td>
      <td><code>path</code>：目标路径编号<br><code>add_node</code>：待追加的节点列表</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>bool add_path_node_single(path_index path, node_index add_node)</code></td>
      <td>追加单个节点到路径末尾</td>
      <td><code>path</code>：目标路径编号<br><code>add_node</code>：待追加的节点编号</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>bool insert_path_node(path_index path, node_pos_get pos, node_index index)</code></td>
      <td>在路径指定位置插入节点</td>
      <td><code>path</code>：目标路径编号<br><code>pos</code>：插入的逻辑位置<br><code>index</code>：待插入的节点编号</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>bool change_path_node(path_index path, node_pos_get pos, node_index index) noexcept</code></td>
      <td>修改路径指定位置的节点编号</td>
      <td><code>path</code>：目标路径编号<br><code>pos</code>：修改的逻辑位置<br><code>index</code>：新的节点编号</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>bool del_path_node(path_index path, std::vector<node_pos_get>& del_node)</code></td>
      <td>批量删除路径中指定位置的节点</td>
      <td><code>path</code>：目标路径编号<br><code>del_node</code>：待删除的逻辑位置列表</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>bool del_path_node_single(path_index path, node_pos_get del_node)</code></td>
      <td>删除路径中指定位置的单个节点</td>
      <td><code>path</code>：目标路径编号<br><code>del_node</code>：待删除的逻辑位置</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>bool exchange_path_part(const path_part& path_one, const path_part& path_sec)</code></td>
      <td>交换两个路径的指定片段（支持同一路径非重叠片段交换）</td>
      <td><code>path_one</code>：第一个路径片段<br><code>path_sec</code>：第二个路径片段</td>
      <td>成功返回true，失败返回false</td>
    </tr>
  </tbody>
</table>

<h3>4. 路径删除接口</h3>

<table>
  <thead>
    <tr>
      <th>函数接口</th>
      <th>功能说明</th>
      <th>参数说明</th>
      <th>返回值</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>bool del_path(path_index path)</code></td>
      <td>删除指定路径，自动释放无引用的节点内存</td>
      <td><code>path</code>：待删除的路径编号</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>void clear()</code></td>
      <td>清空整个共享系统，释放所有路径和节点数据</td>
      <td>无</td>
      <td>无</td>
    </tr>
  </tbody>
</table>

<h3>5. 系统管理接口</h3>

<table>
  <thead>
    <tr>
      <th>函数接口</th>
      <th>功能说明</th>
      <th>参数说明</th>
      <th>返回值</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>bool unify()</code></td>
      <td>全局整合所有路径，最大化节点共享效率，处理内存碎片</td>
      <td>无</td>
      <td>成功返回true，失败返回false</td>
    </tr>
    <tr>
      <td><code>path_state_statistic get_stastic() const noexcept</code></td>
      <td>获取系统状态统计信息（路径数、内存占用、内存使用率等）</td>
      <td>无</td>
      <td>系统状态统计结构体</td>
    </tr>
  </tbody>
</table>

<h3>6. 运算符重载（便捷调用）</h3>

<table>
  <thead>
    <tr>
      <th>运算符接口</th>
      <th>功能说明</th>
      <th>等效原生接口</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>node_index operator()(path_index path, node_pos_get pos) noexcept</code></td>
      <td>快速获取路径指定位置的节点编号</td>
      <td><code>get_path_node_index</code></td>
    </tr>
    <tr>
      <td><code>bool operator()(path_index path, node_pos_get pos, node_index index) noexcept</code></td>
      <td>快速修改路径指定位置的节点编号</td>
      <td><code>change_path_node</code></td>
    </tr>
    <tr>
      <td><code>bool operator()(path_index path_index, path& save) const</code></td>
      <td>快速获取完整路径</td>
      <td><code>get_path</code></td>
    </tr>
  </tbody>
</table>

<h2>📝 完整使用示例</h2>

<h3>示例1：基础路径增删改查</h3>

<pre><code class="language-cpp">using namespace node_path;

// 初始化系统
path_share ps;

// 创建路径 [100]
path_index path = ps.create_path(100);

// 追加节点，路径变为 [100, 200, 300, 400]
ps.add_path_node(path, {200, 300, 400});

// 插入节点，路径变为 [100, 200, 250, 300, 400]
ps.insert_path_node(path, 2, 250);

// 修改节点，路径变为 [100, 200, 250, 350, 400]
ps(path, 3, 350);

// 删除节点，路径变为 [100, 200, 350, 400]
ps.del_path_node_single(path, 2);

// 批量查询节点
std::vector<node_pos_get> positions = {0, 2, 3};
std::vector<node_index> result;
ps.get_path_node_index_mul(path, positions, result);
// result = [100, 350, 400]

// 获取完整路径
path full_path;
ps(path, full_path);
</code></pre>

<h3>示例2：路径分支与共享优化</h3>

<pre><code class="language-cpp">using namespace node_path;

path_share ps;

// 创建主路径 [1,2,3,4,5,6,7,8,9,10]
path_index main_path = ps.create_path(1);
ps.add_path_node(main_path, {2,3,4,5,6,7,8,9,10});

// 创建3条分支路径，共享主路径前缀
path_index branch1 = ps.create_branch(main_path, 4, 101);  // [1,2,3,4,5,101]
path_index branch2 = ps.create_branch(main_path, 6, 201);  // [1,2,3,4,5,6,7,201]
path_index branch3 = ps.create_branch(main_path, 2, 301);  // [1,2,3,301]

// 查看系统状态
path_state_statistic stat = ps.get_stastic();
std::cout << "路径总数：" << stat.quantity_path << std::endl;
std::cout << "内存使用率：" << stat.rate_memory_used * 100 << "%" << std::endl;

// 全局整合，最大化共享效率
ps.unify();

// 删除主路径，分支路径不受影响
ps.del_path(main_path);
</code></pre>

<h2>⚡ 性能指标</h2>

<table>
  <thead>
    <tr>
      <th>操作类型</th>
      <th>时间复杂度</th>
      <th>性能说明</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>单节点路径创建</td>
      <td>O(1)</td>
      <td>单节点创建开销极小</td>
    </tr>
    <tr>
      <td>路径分支创建</td>
      <td>O(K)</td>
      <td>K为分支位置到路径起点的距离，共享节点无拷贝</td>
    </tr>
    <tr>
      <td>单节点随机查询</td>
      <td>O(N/2)</td>
      <td>中点缓存优化，比纯反向链表快50%</td>
    </tr>
    <tr>
      <td>批量节点查询</td>
      <td>O(N/2 + K)</td>
      <td>K为查询数量，一次遍历完成所有查询</td>
    </tr>
    <tr>
      <td>路径编号查找</td>
      <td>O(log M)</td>
      <td>M为路径总数，二分查找实现</td>
    </tr>
    <tr>
      <td>末尾节点追加</td>
      <td>O(1)</td>
      <td>无共享场景下无额外开销</td>
    </tr>
    <tr>
      <td>节点插入/修改/删除</td>
      <td>O(K)</td>
      <td>K为操作位置到路径末尾的距离，仅拷贝需独立化的节点</td>
    </tr>
    <tr>
      <td>路径删除</td>
      <td>O(N)</td>
      <td>N为路径长度，仅释放无引用的节点</td>
    </tr>
    <tr>
      <td>全局共享整合</td>
      <td>O(M*N)</td>
      <td>M为路径总数，N为平均路径长度，建议低频调用</td>
    </tr>
  </tbody>
</table>

<h2>🎯 适用场景</h2>

<ul>
  <li><strong>智能导航系统</strong>：城市道路网络中大量重叠导航路径的存储与管理</li>
  <li><strong>分布式文件系统</strong>：目录树结构的高效存储、硬链接管理</li>
  <li><strong>版本控制系统</strong>：Git等版本工具的分支路径、提交链管理</li>
  <li><strong>图数据库/图算法</strong>：图遍历路径缓存、最短路径预计算</li>
  <li><strong>工作流引擎</strong>：审批流、业务流程的路径管理与版本控制</li>
  <li><strong>嵌入式/边缘设备</strong>：内存资源受限的场景，极致的内存利用率</li>
</ul>

<h2>⚠️ 注意事项</h2>

<ul>
  <li>本系统<strong>非线程安全</strong>，多线程并发场景下需自行添加读写锁保护</li>
  <li>写操作（插入、修改、删除）会触发节点独立化，路径重叠率越高，写放大越明显</li>
  <li>超长路径的随机访问性能有限，更适合批量查询、顺序访问场景</li>
  <li>自定义节点数据需业务层自行维护编号与数据的映射关系</li>
  <li>定期调用 <code>unify()</code> 可最大化共享效率，处理内存碎片，建议空闲时段执行</li>
  <li>批量查询接口要求传入的位置列表为<strong>升序排列</strong>，否则会导致查询结果错误</li>
</ul>

<h2>📄 许可证</h2>

<p>本项目采用 <strong>MIT License</strong> 开源许可证，可自由使用、修改和分发。</p>

<h2>🤝 贡献指南</h2>

<p>欢迎提交 Issue 反馈问题，也欢迎提交 Pull Request 参与代码贡献！</p>

<ol>
  <li>Fork 本仓库</li>
  <li>创建你的功能分支</li>
  <li>提交你的修改</li>
  <li>推送到分支</li>
  <li>创建 Pull Request</li>
</ol>
