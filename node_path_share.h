#pragma once
#include <vector>
#include <algorithm>
#include <cstdint>
#include <unordered_map>

namespace node_path
{

	//基本数据长度
	using data_type_base_unsigned = uint64_t;
	using data_type_base_signed = int64_t;

	using node_index = data_type_base_unsigned;//节点编号
	using node_pos = data_type_base_unsigned;//节点真实储存位置
	using node_pos_get = data_type_base_unsigned;//节点在路径中的位置
	using node_ref = data_type_base_unsigned;//节点被引用次数

	using path_index = data_type_base_unsigned;//路径编号
	using path_pos = data_type_base_unsigned;//路径信息真实储存位置
	using path_len = data_type_base_unsigned;//路径长度

	constexpr node_index NODE_INVALID_INDEX = 0xFFFFFFFFFFFFFFFF;//无效的节点编号
	constexpr node_pos NODE_INVALID_POS = 0xFFFFFFFFFFFFFFFF;//无效的节点储存位置
	constexpr node_pos_get NODE_INVALID_POS_GET = 0xFFFFFFFFFFFFFFFF;//无效的路径节点位置
	constexpr node_ref NODE_INVALID_REF = 0xFFFFFFFFFFFFFFFF;//无效的节点引用次数
	constexpr node_pos NODE_ROOT_POS = 0xFFFFFFFFFFFFFFFE;//根节点位置

	constexpr path_index PATH_INVALID_INDEX = 0xFFFFFFFFFFFFFFFF;//无效的路径编号
	constexpr path_pos PATH_INVALID_POS = 0xFFFFFFFFFFFFFFFF;//无效的路径信息储存位置
	constexpr path_len PATH_INVALID_LEN = 0xFFFFFFFFFFFFFFFF;//无效的路径长度


	//前驱储存结构
	struct ahead
	{
		node_pos last;//上一个前驱储存位置
		node_index index;//此次节点编号
		node_ref ref;//此次被引用次数
		ahead() :
			last(), index(), ref()
		{

		}
		ahead(node_pos last_, node_index index_, node_ref ref_) :
			last(last_), index(index_), ref(ref_)
		{

		}
		ahead(const ahead& ahead_) :
			last(ahead_.last), index(ahead_.index), ref(ahead_.ref)
		{

		}
		bool test_validity_index() const
		{
			return this->index != NODE_INVALID_INDEX;
		}
		bool test_validity_ref() const
		{
			return this->ref != NODE_INVALID_REF;
		}
	};

	//路径基本信息结构
	struct path_info_base
	{
		node_pos last_pos;//最后一个前驱储存位置
		node_pos mid_pos;//中间位置节点所在前驱的位置
		path_len len;//路径长度
		path_info_base() :
			last_pos(), mid_pos(), len()
		{

		}
		path_info_base(node_pos last_pos_, node_pos mid_pos_, path_len len_) :
			last_pos(last_pos_), mid_pos(mid_pos_), len(len_)
		{

		}
		path_info_base(const path_info_base& path_info_base_) :
			last_pos(path_info_base_.last_pos), mid_pos(path_info_base_.mid_pos), len(path_info_base_.len)
		{

		}
		bool get_validity_len() const
		{
			return this->len != PATH_INVALID_LEN;
		}
	};

	//路径信息
	struct path_info
	{
		path_index index;//路径编号
		path_info_base info;//基本信息
		path_info() :
			index(), info()
		{

		}
		path_info(path_index index_, const path_info_base& info_) :
			index(index_), info(info_)
		{

		}
		bool get_validity_index() const
		{
			return this->index != PATH_INVALID_INDEX;
		}
	};

	//路径结构
	struct path
	{
		path_index index;//路径编号
		std::vector<node_index> nodes;//路径所有节点
		path() :
			index(), nodes()
		{

		}
		path(path_index index_, const std::vector<node_index>& nodes_) :
			index(index_), nodes(nodes_)
		{

		}
		path(path_index index_, std::vector<node_index>&& nodes_) :
			index(index_), nodes(std::move(nodes_))
		{

		}
		path(path&& path_) noexcept :
			index(path_.index), nodes(std::move(path_.nodes))
		{
			path_.index = PATH_INVALID_INDEX;
		}
		path& operator=(const path& path_)
		{
			this->index = path_.index;
			this->nodes = path_.nodes;
			return *this;
		}
	};

	//路径部分区间结构
	struct path_part
	{
		path_index index;//路径编号
		node_pos_get pos_start;//起点
		node_pos_get pos_end;//终点
		path_part() :
			index(), pos_start(), pos_end()
		{

		}
		path_part(path_index index_, node_pos_get pos_start_, node_pos_get pos_end_) :
			index(index_), pos_start(pos_start_), pos_end(pos_end_)
		{

		}
	};

	//路径整合节点构造结构
	struct path_unify_construct
	{
		node_pos last;//上一个前驱储存位置
		node_index current;//此次节点编号
		path_unify_construct() :
			last(), current()
		{

		}
		path_unify_construct(node_pos last_, node_index current_) :
			last(last_), current(current_)
		{

		}
		bool operator==(const path_unify_construct& path_unify_construct_other) const
		{
			return path_unify_construct_other.last == this->last && path_unify_construct_other.current == this->current;
		}
	};

	//路径整合哈希
	struct path_unify_hash
	{
		size_t operator()(const path_unify_construct& unify_construct_) const {
			const data_type_base_unsigned prime = 0x9e3779b97f4a7c15ULL;

			data_type_base_unsigned hash = unify_construct_.last;
			hash ^= unify_construct_.current + prime + (hash << 6) + (hash >> 2);

			hash = (~hash) + (hash << 21);
			hash = hash ^ (hash >> 24);
			hash = (hash + (hash << 3)) + (hash << 8);
			hash = hash ^ (hash >> 14);
			hash = (hash + (hash << 2)) + (hash << 4);
			hash = hash ^ (hash >> 28);
			hash = hash + (hash << 31);

			return static_cast<size_t>(hash);
		}
	};

	//路径系统状态统计结构
	struct path_state_statistic
	{
		size_t quantity_path;//路径数量
		size_t quantity_bytes_memory_alloc;//已分配的内存字节数
		size_t quantity_bytes_memory_used;//实际有效使用的内存字节数
		float rate_memory_used;//内存使用率
		path_state_statistic() :
			quantity_path(), quantity_bytes_memory_alloc(), quantity_bytes_memory_used(), rate_memory_used()
		{

		}
		path_state_statistic(size_t quantity_path_, size_t quantity_bytes_memory_alloc_, size_t quantity_bytes_memory_used_, float rate_memory_used_) :
			quantity_path(quantity_path_), quantity_bytes_memory_alloc(quantity_bytes_memory_alloc_), quantity_bytes_memory_used(quantity_bytes_memory_used_), rate_memory_used(rate_memory_used_)
		{

		}
	};

	//节点
	template<typename node_data>
	class node
	{
	private:
		node_index index;//节点编号
		node_data data;//数据
	public:
		node() :
			index(NODE_INVALID_INDEX), data()
		{

		}
		node(const node_data& data_) :
			index(NODE_INVALID_INDEX), data(data_)
		{

		}
		node(node_data&& data_) noexcept:
			index(NODE_INVALID_INDEX), data(std::move(data_))
		{

		}
		node(node_index index_, const node_data& data_) :
			index(index_), data(data_)
		{

		}
		node(node_index index_, node_data&& data_) :
			index(index_), data(std::move(data_))
		{

		}
		~node()
		{

		}
	public:
		node_index get_index() const noexcept
		{
			return this->index;
		}
		node_data& get_data_ref() noexcept
		{
			return this->data;
		}
		const node_data& get_const_data_ref() const noexcept
		{
			return this->data;
		}
		bool get_validity_node_index() const noexcept
		{
			return this->index != NODE_INVALID_INDEX;
		}
	public:
		bool set_index(node_index index) noexcept
		{
			if (index == NODE_INVALID_INDEX)
				return false;

			this->index = index;
			return true;
		}
		void set_data(const node_data& data)
		{
			this->data = data;
			return;
		}
		void set_data(node_data&& data) noexcept
		{
			this->data = std::move(data);
			return;
		}
	};

    //路径共享
	class path_share
	{
	private:
		std::vector<ahead> node_set;//前驱集合
		std::vector<node_pos> node_set_free;//空闲前驱列表，记录由于删除造成的空洞，留作之后重复使用
		std::vector<path_info> path_set;//路径信息集合，这里按照路径编号进行升序储存，且不留空洞
		std::vector<path_index> path_index_free;//空闲路径编号列表
	private:
		//检测节点编号是否有效
		bool test_validity_node_index(node_index index) const noexcept
		{
			return index != NODE_INVALID_INDEX;
		}
		//检测前驱位置是否有效
		bool test_validity_node_pos(node_pos pos) const noexcept
		{
			if (pos == NODE_INVALID_POS)
				return false;

			if (pos == NODE_ROOT_POS)
				return true;

			if (pos >= this->node_set.size())
				return false;

			node_pos target = this->node_set[pos].last;

			return target != NODE_INVALID_POS && (target < this->node_set.size() || target == NODE_ROOT_POS);//确保前驱未被删除
		}
		//检测路径中的位置是否有效
		bool test_validity_node_pos_get(path_index path, node_pos_get pos) const noexcept
		{
			const path_info* path_info_target = get_const_path_info_by_path_pos_ptr(get_path_pos(path));

			return path_info_target == nullptr ? false : pos < path_info_target->info.len;//确保路径存在且节点位置在路径中
		}
		//检测路径编号是否有效
		bool test_validity_path_index(path_index index) const noexcept
		{
			if (index == PATH_INVALID_INDEX)
				return false;

			return get_path_pos(index) != PATH_INVALID_POS;
		}
		//检测路径位置是否有效
		bool test_validity_path_pos(path_pos pos) const noexcept
		{
			return pos != PATH_INVALID_POS && pos < this->path_set.size();//由于路径信息的储存没有空洞，只要在范围大小内，一定有效
		}
		//获取全部路径信息的常量引用
		const std::vector<path_info>& get_const_path_set_ref() const noexcept
		{
			return this->path_set;
		}
		//获取空闲的前驱位置
		node_pos get_free_node_set() noexcept
		{
			if (this->node_set_free.empty() == false)
			{
				node_pos free = this->node_set_free.back();
				this->node_set_free.pop_back();
				return free;//直接取用空闲列表中的最后一个
			}

			return NODE_INVALID_POS;//没有空闲返回无效值
		}
		//获取空闲路径编号
		path_index get_free_path_index() noexcept
		{
			if (this->path_index_free.empty() == false)
			{
				path_index free = this->path_index_free.back();
				this->path_index_free.pop_back();
				return free;//直接取用空闲列表中的最后一个
			}

			return PATH_INVALID_INDEX;//没有空闲返回无效值
		}
		//获取路径信息
		path_info get_path_info_by_path_pos(path_pos pos) const noexcept
		{
			if (test_validity_path_pos(pos) == false)
				return path_info(PATH_INVALID_INDEX, path_info_base(NODE_INVALID_POS, NODE_INVALID_POS, PATH_INVALID_LEN));//没有这样的路径就返回无效的路径信息

			return this->path_set[pos];
		}
		//获取路径信息的指针
		path_info* get_path_info_by_path_pos_ptr(path_pos pos) noexcept
		{
			if (test_validity_path_pos(pos) == false)
				return nullptr;//请求位置无效，返回空指针

			return &this->path_set[pos];
		}
		//获取路径信息的常量指针
		const path_info* get_const_path_info_by_path_pos_ptr(path_pos pos) const noexcept
		{
			if (test_validity_path_pos(pos) == false)
				return nullptr;//请求位置无效，，返回空指针

			return &this->path_set[pos];
		}
		//获取指定前驱的指针
		ahead* get_ahead_by_node_pos(node_pos pos) noexcept
		{
			if (pos == NODE_ROOT_POS)
				return nullptr;//请求位置无效，返回空指针
			if (test_validity_node_pos(pos) == false)
				return nullptr;//请求位置无效，返回空指针

			return &this->node_set[pos];
		}
		//获取指定前驱的常量指针
		const ahead* get_const_ahead_by_node_pos(node_pos pos) const noexcept
		{
			if (pos == NODE_ROOT_POS)
				return nullptr;//请求位置无效，返回空指针
			if (test_validity_node_pos(pos) == false)
				return nullptr;//请求位置无效，返回空指针

			return &this->node_set[pos];
		}
		//获取路径位置
		path_pos get_path_pos(path_index index) const noexcept
		{
			if (index == PATH_INVALID_INDEX)
				return PATH_INVALID_POS;//请求路径编号无效，返回无效位置

			std::vector<path_info>::const_iterator pos_begin = this->path_set.begin();
			std::vector<path_info>::const_iterator pos_target = std::lower_bound(
				pos_begin,
				this->path_set.end(),
				index,
				[](const path_info& path, path_index index_target)->bool
				{
					return path.index < index_target;
				}
			);//由于路径信息为按编号升序储存，这里直接二分查找

			return pos_target->index == index ? static_cast<path_pos>(pos_target - pos_begin) : PATH_INVALID_POS;//需最后确认二分查找到的确实为所请求的路径
		}
		//批量获取用节点在路径中的位置获取节点的真实位置
		bool get_node_pos_by_node_pos_get_mul(const path_info* path_info_ptr, const std::vector<node_pos_get>& pos, std::vector<node_pos>& save) const//pos升序
		{	

			//因为路径节点间的连接是反向的，所以这里的位置请求集合，也就是pos，这里要求升序储存，以便从集合的末尾开始遍历


			save.clear();//先清空结果保存容器

			if (pos.empty() == true)//请求位置
				return false;

			if (path_info_ptr == nullptr)//无效的路径信息
				return false;

			save.resize(pos.size(), NODE_INVALID_POS);//先提前分配保存空间，同时全部先用无效值填充

			std::vector<node_pos_get>::const_iterator pos_end = pos.end();
			std::vector<node_pos_get>::const_iterator pos_begin = pos.begin();
			std::vector<node_pos_get>::const_iterator pos_seek_begin = std::upper_bound(
				pos_begin,
				pos_end,
				static_cast<node_pos_get>(path_info_ptr->info.len - 1),
				[](node_pos_get pos_one, node_pos_get pos_sec)->bool
				{
					return pos_one < pos_sec;
				}
			);//找出第一个超出范围的请求

			size_t pos_start = static_cast<size_t>(pos_end - pos_seek_begin);//确定开始位置

			if (pos_seek_begin == pos_begin)
				return false;//全都是超出范围的

			pos_seek_begin--;//回到最大的有效请求中

			node_pos_get pos_get_mid = static_cast<node_pos_get>((path_info_ptr->info.len - 1) / 2);//路径中点位置
			node_pos pos_current = NODE_INVALID_POS;//当前储存位置
			node_pos_get pos_get_current = NODE_INVALID_POS_GET;//当前请求路径位置
			size_t len_loop = 0;//循环长度
			bool pos_mid_is_replaced = false;//是否已经使用中点位置

			if (*pos_seek_begin <= pos_get_mid)//判断当前最大的路径位置是否小于等于终点位置
			{
				//如果小于等于中点位置
				pos_current = path_info_ptr->info.mid_pos;//遍历位置直接从中点位置开始
				pos_get_current = pos_get_mid;//当前请求路径位置直接从中点位置开始
				len_loop = static_cast<size_t>(pos_get_mid + 1) - static_cast<size_t>(pos.front());
				pos_mid_is_replaced = true;
			}
			else
			{
				//如果大于中点位置
				pos_current = path_info_ptr->info.last_pos;//从末尾开始
				pos_get_current = static_cast<node_pos_get>(path_info_ptr->info.len - 1);
				len_loop = static_cast<size_t>(path_info_ptr->info.len - pos.front());
			}

			std::vector<node_pos>::iterator pos_save = save.end() - pos_start - 1;//计算开始储存的位置

			while (len_loop > 0)
			{
				while (*pos_seek_begin >= pos_get_current)//这里需要考虑请求位置中有多个相同的，这里直接将相同的请求填充相同结果
				{
					*pos_save = pos_current;

					if (pos_seek_begin == pos_begin || pos_save == save.begin())
						break;

					pos_seek_begin--;
					pos_save--;
				}
				
				if (*pos_seek_begin <= pos_get_mid && pos_mid_is_replaced == false)//这里判断是否需要提前跳转，减少循环次数
				{
					size_t forward_steps = static_cast<size_t>(*(pos_seek_begin + 1) - pos_get_mid - 1);//计算跳跃步数

					pos_current = path_info_ptr->info.mid_pos;
					len_loop -= forward_steps;
					pos_get_current -= static_cast<node_pos_get>(forward_steps);
					pos_mid_is_replaced = true;
				}			
				else
				{
					pos_current = get_const_ahead_by_node_pos(pos_current)->last;//不需要也不能够跳转，直接下一个前驱
				}

				pos_get_current--;
				len_loop--;
			}

			return true;
		}
		//用节点在路径中的位置获取节点的真实位置
		node_pos get_node_pos_by_node_pos_get(const path_info* path_info_ptr, node_pos_get pos) const
		{
			std::vector<node_pos> save;
			bool result = get_node_pos_by_node_pos_get_mul(path_info_ptr, std::move(std::vector<node_pos_get>({ pos })), save);//单位置寻找
			return result == true ? save.front() : NODE_INVALID_POS;
		}
		//自增指定路径从末尾到根节点的引用次数
		bool add_path_part_node_ref_by_node_pos_get(const path_info* path_info_ptr, node_pos_get pos) noexcept
		{
			if (path_info_ptr == nullptr)
				return false;

			ahead* node_current = get_ahead_by_node_pos(path_info_ptr->info.last_pos);
			size_t len_loop = static_cast<size_t>(static_cast<node_pos_get>(path_info_ptr->info.len - 1) - pos);
			size_t len_current = 0;

			while (len_current <= len_loop)
			{
				node_current->ref++;
				node_current = get_ahead_by_node_pos(node_current->last);//下一个前驱
				len_current++;
			}

			return true;
		}
		//自增指定路径中所有节点的引用次数
		bool add_path_node_ref(const path_info* path_info_ptr)
		{
			if (path_info_ptr == nullptr)
				return false;

			return add_path_part_node_ref_by_node_pos_get(path_info_ptr, 0);//整条路径
		}
		//新建路径信息
		path_index create_path_info(const path_info_base& new_path)
		{
			if (new_path.get_validity_len() == false)
				return PATH_INVALID_INDEX;//新信息中存在无效的路径长度

			path_index free = get_free_path_index();//获取空闲路径编号

			if (test_validity_path_index(free) == false)
			{
				//没有空闲的编号
				path_index new_ptah_index = this->path_set.empty() == false ? this->path_set.back().index + 1 : 0;//已有路径，则最大编号+1；没有路径，从0开始
				this->path_set.emplace_back(new_ptah_index, new_path);
				return new_ptah_index;
			}

			std::vector<path_info>::iterator pos_insert = std::upper_bound(
				this->path_set.begin(),
				this->path_set.end(),
				free,
				[](path_index index_target, const path_info& path)->bool
				{
					return index_target < path.index;
				}
			);//路径信息升序储存，这里查找插入位置

			this->path_set.insert(pos_insert, path_info(free, new_path));
			return free;
		}
		//新建前驱储存
		node_pos create_ahead(const ahead& new_ahead)
		{
			if (test_validity_node_pos(new_ahead.last) == false)
				return NODE_INVALID_POS;//新前驱指向的位置无效
			if (new_ahead.test_validity_index() == false)
				return NODE_INVALID_POS;//新前驱的节点编号无效
			if (new_ahead.test_validity_ref() == false)
				return NODE_INVALID_POS;//新前驱的引用次数无效

			node_pos free = get_free_node_set();//获取空闲位置
			node_pos path_new_last_pos = NODE_INVALID_POS;

			if (test_validity_node_index(free) == false)
			{
				//没有空闲位置
				this->node_set.emplace_back(new_ahead);
				path_new_last_pos = static_cast<node_pos>(this->node_set.size() - 1);
			}
			else
			{
				//有空闲位置，直接复制信息
				this->node_set[free] = new_ahead;
				path_new_last_pos = free;
			}

			return path_new_last_pos;
		}
		//用路径位置删除路径信息
		bool del_path_info_by_path_pos(path_pos del_pos)
		{
			path_info* path_info_target = get_path_info_by_path_pos_ptr(del_pos);
			
			if (path_info_target == nullptr)
				return false;//无效的路径
			
			path_info_target->info = path_info_base(NODE_INVALID_POS, NODE_INVALID_POS, PATH_INVALID_LEN);//设置为无效信息
			this->path_index_free.emplace_back(path_info_target->index);
			this->path_set.erase(this->path_set.begin() + del_pos);
			return true;
		}
		//用节点位置删除前驱储存
		bool del_ahead_by_node_pos(node_pos del_pos)
		{
			ahead* ahead_target = get_ahead_by_node_pos(del_pos);

			if (ahead_target == nullptr)
				return false;

			//设置无效信息
			ahead_target->last = NODE_INVALID_POS;
			ahead_target->index = NODE_INVALID_INDEX;
			ahead_target->ref = NODE_INVALID_REF;

			this->node_set_free.emplace_back(del_pos);
			return true;
		}
		//独立路径中的部分
		bool make_path_unique_part(path_info* path_info_ptr, node_pos_get pos)
		{
			if (path_info_ptr == nullptr)
				return false;//无效的路径

			if (pos >= static_cast<node_pos_get>(path_info_ptr->info.len))
				return false;

			node_pos_get pos_get_mid_old = static_cast<node_pos_get>((path_info_ptr->info.len - 1) / 2);//路径中点位置
			node_pos pos_mid = pos <= pos_get_mid_old ? NODE_INVALID_POS : path_info_ptr->info.mid_pos;//新储存中点位置，如果要独立的位置不超过中点，直接复用原先的位置即可
			node_pos pos_last = NODE_INVALID_POS;//上一个位置
			node_pos pos_current = path_info_ptr->info.last_pos;//当前的位置
			node_pos pos_info_new = path_info_ptr->info.last_pos;//新的路径末尾入口位置
			ahead* node_current = get_ahead_by_node_pos(pos_current);//当前的前驱

			//处理末尾前驱
			if (node_current->ref >= 2)
			{
				//末尾有多处引用，需要进行独立化

				pos_current = node_current->last;//先写入下一个前驱
				node_current->ref--;
				node_pos pos_new = create_ahead(ahead(pos_current, node_current->index, 1));//新建前驱

				if (pos_mid == NODE_INVALID_POS)
				{
					if (static_cast<node_pos_get>(path_info_ptr->info.len / 2) == 0)
						pos_mid = pos_new;//末尾节点所在位置是中点位置
				}

				pos_info_new = pos_new;
				pos_last = pos_new;
				node_current = get_ahead_by_node_pos(pos_current);
			}
			else
			{

				if (pos_mid == NODE_INVALID_POS)
				{
					if (static_cast<node_pos_get>(path_info_ptr->info.len / 2) == 0)
						pos_mid = path_info_ptr->info.mid_pos;//末尾节点所在位置是中点位置
				}

				pos_last = pos_current;
				pos_current = node_current->last;
				node_current = get_ahead_by_node_pos(pos_current);
			}

			size_t len_loop = static_cast<size_t>(static_cast<node_pos_get>(path_info_ptr->info.len - pos) - 1);
			node_pos_get pos_get_current = static_cast<node_pos_get>(path_info_ptr->info.len - 1);

			while (len_loop > 0)
			{
				node_pos pos_last_temp = node_current->last;
				if (node_current->ref >= 2)
				{
					//处理多引用前驱

					node_current->ref--;
					node_pos pos_new = create_ahead(ahead(node_current->last, node_current->index, 1));//建立新前驱

					if (test_validity_node_pos(pos_last) == true)
						get_ahead_by_node_pos(pos_last)->last = pos_new;

					pos_last = pos_new;
				}
				else
				{
					pos_last = pos_current;
				}

				pos_current = pos_last_temp;
				node_current = get_ahead_by_node_pos(pos_last_temp);
				pos_get_current--;
				len_loop--;

				if (pos_mid == NODE_INVALID_POS)
				{
					if(pos_get_current == pos_get_mid_old)
						pos_mid = pos_last;//进入循环处理前不能获得新中点位置，需要在此获取。这里遍历到了中点位置，直接写入
				}
			}

			path_info_ptr->info = path_info_base(pos_info_new, pos_mid, path_info_ptr->info.len);//更新路径信息
			return true;
		}
		//独立路径
		bool make_path_unique(path_info* path_info_ptr)
		{
			return make_path_unique_part(path_info_ptr, 0);//独立化整条路径
		}
		//获取单条路径的节点
		bool get_path_single(const path_info* path_info_ptr, path& save) const
		{
			save.nodes.clear();

			if (path_info_ptr == nullptr)
				return false;

			save.nodes.resize(static_cast<size_t>(path_info_ptr->info.len));

			const ahead* ahead_current = get_const_ahead_by_node_pos(path_info_ptr->info.last_pos);
			size_t len_loop = static_cast<size_t>(path_info_ptr->info.len);
			size_t len_current = 0;

			//开始遍历路径
			while (len_current < len_loop)
			{
				save.nodes[len_loop - len_current - 1] = ahead_current->index;

				if (test_validity_node_pos(ahead_current->last) == false)
				{
					save.nodes.clear();
					return false;
				}

				ahead_current = get_const_ahead_by_node_pos(ahead_current->last);
				len_current++;
			}

			save.index = path_info_ptr->index;
			return true;			
		}
		//整合路径节点，最大化共享
		bool unify_node_set()
		{
			std::vector<path> path_all;
			if (get_path_all(path_all) == false)//先获取全部路径的全部节点
				return false;

			std::vector<ahead> node_set_new;
			size_t node_ref_total = 0;
			for (std::vector<path>::const_iterator path_each = path_all.begin(); path_each != path_all.end(); path_each++)
				node_ref_total += path_each->nodes.size();
			node_set_new.reserve(node_ref_total);

			std::unordered_map<path_unify_construct, node_pos, path_unify_hash> node_unify_pos;
			node_unify_pos.reserve(node_ref_total);

			std::vector<std::pair<path_index, path_info_base>> path_info_new;
			path_info_new.reserve(path_all.size());

			for (std::vector<path>::const_iterator path_each = path_all.begin(); path_each != path_all.end(); path_each++)
			{
				if (path_each->nodes.empty() == true)
					return false;

				node_pos pos_last = NODE_ROOT_POS;
				node_pos pos_mid = NODE_INVALID_POS;
				const size_t pos_mid_target = (path_each->nodes.size() - 1) / 2;

				for (size_t pos_current = 0; pos_current < path_each->nodes.size(); pos_current++)
				{
					path_unify_construct node_key(pos_last, path_each->nodes[pos_current]);
					std::unordered_map<path_unify_construct, node_pos, path_unify_hash>::iterator node_find = node_unify_pos.find(node_key);
					node_pos pos_new = NODE_INVALID_POS;

					if (node_find == node_unify_pos.end())
					{
						pos_new = static_cast<node_pos>(node_set_new.size());
						node_set_new.emplace_back(pos_last, path_each->nodes[pos_current], 0);
						node_unify_pos.emplace(node_key, pos_new);
					}
					else
					{
						pos_new = node_find->second;
					}

					node_set_new[pos_new].ref++;

					if (pos_current == pos_mid_target)
						pos_mid = pos_new;

					pos_last = pos_new;
				}

				if (pos_mid == NODE_INVALID_POS || pos_last == NODE_INVALID_POS)
					return false;

				path_info_new.emplace_back(path_each->index, path_info_base(pos_last, pos_mid, static_cast<path_len>(path_each->nodes.size())));
			}

			for (std::vector<ahead>::const_iterator node_each = node_set_new.begin(); node_each != node_set_new.end(); node_each++)
			{
				if (node_each->test_validity_index() == false || node_each->test_validity_ref() == false)
					return false;
				if (node_each->last != NODE_ROOT_POS && node_each->last >= node_set_new.size())
					return false;
			}

			for (std::vector<std::pair<path_index, path_info_base>>::const_iterator path_each = path_info_new.begin(); path_each != path_info_new.end(); path_each++)
			{
				if (test_validity_path_pos(get_path_pos(path_each->first)) == false)
					return false;
			}

			this->node_set = std::move(node_set_new);
			this->node_set_free.clear();

			for (std::vector<std::pair<path_index, path_info_base>>::const_iterator path_each = path_info_new.begin(); path_each != path_info_new.end(); path_each++)
			{
				path_info* path_info_target = get_path_info_by_path_pos_ptr(get_path_pos(path_each->first));
				path_info_target->info.last_pos = path_each->second.last_pos;
				path_info_target->info.mid_pos = path_each->second.mid_pos;
				path_info_target->info.len = path_each->second.len;
			}

			return true;
		}
		//获取系统状态
		path_state_statistic get_state() const noexcept
		{
			//总字节分配数
			size_t alloc_total =
				this->path_set.size() * sizeof(path_info) +
				this->path_index_free.size() * sizeof(path_index) +
				this->node_set.size() * sizeof(ahead) +
				this->node_set_free.size() * sizeof(node_pos);

			//已使用的字节数
			size_t used_total =
				this->path_set.size() * sizeof(path_info) +
				this->path_index_free.size() * sizeof(path_index) +
				(this->node_set.size() - this->node_set_free.size()) * sizeof(ahead) +
				this->node_set_free.size() * sizeof(node_pos);

			float alloc_total_float = static_cast<float>(alloc_total);
			float used_total_float = static_cast<float>(used_total);

			return path_state_statistic(
				this->path_set.size(),
				alloc_total,
				used_total,
				used_total_float / alloc_total_float
			);
		}
		//清空系统
		void clear_all()
		{
			//全部清空
			this->node_set.clear();
			this->node_set_free.clear();
			this->path_set.clear();
			this->path_index_free.clear();
		}
	public:
		path_share() :
			node_set(), node_set_free(), path_set(), path_index_free()
		{

		}
		path_share(const path_share& path_share_) :
			node_set(path_share_.node_set), node_set_free(path_share_.node_set_free), path_set(path_share_.path_set), path_index_free(path_share_.path_index_free)
		{

		}
		path_share(path_share&& path_share_) noexcept :
			node_set(std::move(path_share_.node_set)), 
			node_set_free(std::move(path_share_.node_set_free)), 
			path_set(std::move(path_share_.path_set)), 
			path_index_free(std::move(path_share_.path_index_free))
		{

		}
		~path_share() noexcept
		{

		}
	public:
		//整合全部路径使其最大化共享，处理内存碎片
		bool unify()
		{
			return unify_node_set();
		}
		//获取路径多个指定位置的节点编号
		bool get_path_node_index_mul(path_index path, const std::vector<node_pos_get>& pos_mul, std::vector<node_index>& save) const
		{
			save.clear();

			const path_info* path_info_target = get_const_path_info_by_path_pos_ptr(get_path_pos(path));

			if (path_info_target == nullptr)
				return NODE_INVALID_INDEX;//无效的路径

			std::vector<node_pos> save_target;

			if (get_node_pos_by_node_pos_get_mul(path_info_target, pos_mul, save_target) == false)
				return false;

			save.resize(pos_mul.size());

			std::vector<node_pos>::iterator pos_current = save_target.begin();
			std::vector<node_index>::iterator pos_save_current = save.begin();

			while (pos_current != save_target.end())
			{
				*pos_save_current = get_const_ahead_by_node_pos(*pos_current)->index;
				pos_current++;
				pos_save_current++;
			}

			return true;
		}
		//获取路径指定位置的节点编号
		node_index get_path_node_index(path_index path, node_pos_get pos) const noexcept
		{
			const path_info* path_info_target = get_const_path_info_by_path_pos_ptr(get_path_pos(path));

			if (path_info_target == nullptr)
				return NODE_INVALID_INDEX;//无效的路径

			const ahead* ahead_target = get_const_ahead_by_node_pos(get_node_pos_by_node_pos_get(path_info_target, pos));
			return ahead_target == nullptr ? NODE_INVALID_INDEX : ahead_target->index;//前驱获取失败：返回无效值；成功：直接读取返回
		}
		//获取路径长度
		path_len get_path_len(path_index path) const noexcept
		{
			const path_info* info_target = get_const_path_info_by_path_pos_ptr(get_path_pos(path));

			return info_target == nullptr ? PATH_INVALID_LEN : info_target->info.len;
		}
		//获取系统中的路径数量
		size_t get_path_quantity() const noexcept
		{
			return this->path_set.size();
		}
		//获取指定路径
		bool get_path(path_index path_index, path& save) const
		{
			const path_info* path_target = get_const_path_info_by_path_pos_ptr(get_path_pos(path_index));

			return get_path_single(path_target, save);
		}
		//获取全部路径
		bool get_path_all(std::vector<path>& save) const
		{
			const std::vector<path_info>& path_set_target = get_const_path_set_ref();

			save.resize(path_set_target.size());

			std::vector<path_info>::const_iterator path_current = path_set_target.begin();
			std::vector<path>::iterator path_save_current = save.begin();

			//开始遍历所有路径
			while (path_current != path_set_target.end())
			{			
				if (get_path_single(&*path_current, *path_save_current) == false)
					return false;
				path_current++;
				path_save_current++;
			}

			return true;
		}
		//新建路径
		path_index create_path(node_index index)
		{
			node_pos pos_ahead_new = create_ahead(ahead(NODE_ROOT_POS, index, 1));

			if (test_validity_node_pos(pos_ahead_new) == false)
				return PATH_INVALID_INDEX;

			path_index path_new = create_path_info(path_info_base(pos_ahead_new, pos_ahead_new, 1));//新建路径信息

			if (test_validity_path_index(path_new) == false)
			{
				del_ahead_by_node_pos(pos_ahead_new);//新建失败，删除新建的前驱
				return PATH_INVALID_INDEX;//新建失败，返回无效值
			}

			return path_new;
		}
		//新建分支
		path_index create_branch(path_index path, node_pos_get make_pos, node_index index)
		{
			path_info* path_info_target = get_path_info_by_path_pos_ptr(get_path_pos(path));
			std::vector<node_pos> save;

			//这里由于要更新中点位置，要同时获取新中点位置和目的位置
			if (get_node_pos_by_node_pos_get_mul(path_info_target, std::move(std::vector<node_pos_get>({ (make_pos + 1) / 2,make_pos })), save) == false)
				return PATH_INVALID_INDEX;//获取失败

			node_pos pos_branch = save.back();
			node_pos pos_ahead_new = create_ahead(ahead(pos_branch, index, 1));

			if (test_validity_node_pos(pos_ahead_new) == false)
				return PATH_INVALID_INDEX;//新建前驱失败

			path_index path_new = create_path_info(path_info_base(pos_ahead_new, save.front(), static_cast<path_len>(make_pos + 2)));

			if (test_validity_path_index(path_new) == false)
			{
				del_ahead_by_node_pos(pos_ahead_new);//新建路径信息失败，删除新建的前驱
				return PATH_INVALID_INDEX;
			}

			path_pos pos_path_new = get_path_pos(path_new);

			path_info_target = get_path_info_by_path_pos_ptr(pos_path_new);

			if (add_path_part_node_ref_by_node_pos_get(path_info_target, make_pos) == false)
			{
				//引用增加失败，删除所有新建的
				del_ahead_by_node_pos(pos_ahead_new);
				del_path_info_by_path_pos(pos_path_new);
				return PATH_INVALID_INDEX;
			}

			return path_new;
		}
		//向路径的指定位置插入节点编号
		bool insert_path_node(path_index path, node_pos_get pos, node_index index)
		{
			if (test_validity_node_index(index) == false)
				return false;//插入的节点编号无效

			path_info* path_target = get_path_info_by_path_pos_ptr(get_path_pos(path));

			if (path_target == nullptr)
				return false;//无效的路径

			if (make_path_unique_part(path_target, pos) == false)
				return false;//独立化失败
			
			node_pos pos_new = create_ahead(ahead(NODE_ROOT_POS, index, 1));

			if (test_validity_node_pos(pos_new) == false)
				return false;//新建前驱失败

			ahead* ahead_new = get_ahead_by_node_pos(pos_new);
				
			node_pos_get pos_get_mid_new = static_cast<node_pos_get>(path_target->info.len / 2);//新的路径中点位置
			node_pos pos_mid_new = NODE_INVALID_POS;
			node_pos pos_insert = NODE_INVALID_POS;

			if (pos == pos_get_mid_new)
			{
				pos_insert = get_node_pos_by_node_pos_get(path_target, pos);
				pos_mid_new = pos_new;//插入位置刚好是新的路径中点位置，直接写入
			}
			else
			{
				//插入位置不是新的路径中点位置，需要考虑插入的位置是在其前面还是后面

				std::vector<node_pos> save;
				bool pos_insert_is_bigger = pos > pos_get_mid_new;

				if (get_node_pos_by_node_pos_get_mul(
					path_target,
					std::move(
						std::vector<node_pos_get>
						(
							{  
								pos_insert_is_bigger ? pos_get_mid_new : pos,
							    pos_insert_is_bigger ? pos : pos_get_mid_new - 1
							}
						)
					), 
					save
				) == false)
				{
					del_ahead_by_node_pos(pos_new);
					return false;
				}

				pos_mid_new = pos_insert_is_bigger ? save.front() : save.back();
				pos_insert = pos_insert_is_bigger ? save.back() : save.front();
			}
		
			if (test_validity_node_pos(pos_insert) == false)
			{
				del_ahead_by_node_pos(pos_new);
				return false;
			}

			ahead* ahead_insert = get_ahead_by_node_pos(pos_insert);

			ahead_new->last = ahead_insert->last;
			ahead_insert->last = pos_new;
			path_target->info.len += 1;
			path_target->info.mid_pos = pos_mid_new;
			return true;
		}
		//在路径的末尾批量添加节点编号
		bool add_path_node(path_index path, const std::vector<node_index>& add_node)
		{
			if (add_node.empty() == true)
				return true;//空的请求

			path_info* path_target_info = get_path_info_by_path_pos_ptr(get_path_pos(path));

			if (path_target_info == nullptr)
				return false;//无效的路径

			node_pos pos_ahead_last = create_ahead(ahead(path_target_info->info.last_pos, add_node.front(), 1));

			if (test_validity_node_pos(pos_ahead_last) == false)
				return false;//新建前驱失败

			node_pos_get pos_get_mid_new = static_cast<node_pos_get>((static_cast<size_t>(path_target_info->info.len) + add_node.size() - 1) / 2);//新的路径中点位置
			node_pos pos_mid_new = NODE_INVALID_POS;

			//如果新的路径中点位置仍在原路径范围内
			if (pos_get_mid_new < path_target_info->info.len)
			{
				pos_mid_new = get_node_pos_by_node_pos_get(path_target_info, pos_get_mid_new);

				if (test_validity_node_pos(pos_mid_new) == false)
				{
					del_ahead_by_node_pos(pos_ahead_last);
					return false;
				}
			}

			std::vector<node_index>::const_iterator add_target = add_node.begin() + 1;
			node_pos_get pos_mid_target = pos_get_mid_new - static_cast<node_pos_get>(path_target_info->info.len);//如果如果新的路径中点位置超出原路径范围，这里计算多少步后达到目标位置

			//开始遍历加入
			while (add_target != add_node.end())
			{
				if (pos_mid_new == NODE_INVALID_POS)
				{
					if (pos_mid_target == 0) pos_mid_new = pos_ahead_last;//达到目标位置，写入新的路径中点位置
					else pos_mid_target--;
				}

				pos_ahead_last = create_ahead(ahead(pos_ahead_last, *add_target, 1));				
				add_target++;		
			}

			path_target_info->info = path_info_base(pos_ahead_last, pos_mid_new, path_target_info->info.len + add_node.size());
			return true;
		}
		//在路径的末尾添加单个节点编号
		bool add_path_node_single(path_index path, node_index add_node)
		{
			return add_path_node(path, std::move(std::vector<node_index>({ add_node })));//单节点加入
		}
		//交换一个或两个路径的片段
		bool exchange_path_part(const path_part& path_one, const path_part& path_sec)
		{
			//前位置不得大于后位置

			if (path_one.pos_end < path_one.pos_start)
				return false;
			if (path_sec.pos_end < path_sec.pos_start)
				return false;

			//以下常量用于获取位置
			constexpr size_t end_last_index = 5;//交换结束的下一个位置的节点编号
			constexpr size_t end_index = 4;//交换结束的节点编号
			constexpr size_t end_tail_index = 3;//交换结束的上一个位置的节点编号
			constexpr size_t start_last_index = 2;//交换起点的下一个位置的节点编号
			constexpr size_t start_index = 1;//交换起点的节点编号
			constexpr size_t start_tail_index = 0;//交换起点的上一个位置的节点编号

			//交换路径是否为同一路径
			if (path_one.index == path_sec.index)
			{
				if (path_one.pos_start <= path_sec.pos_end && path_sec.pos_start <= path_one.pos_end)
					return false;//存在重叠

				path_info* path_info_target = get_path_info_by_path_pos_ptr(get_path_pos(path_one.index));

				if (path_info_target == nullptr)
					return false;

				if (make_path_unique_part(path_info_target, path_one.pos_start < path_sec.pos_start ? path_one.pos_start : path_sec.pos_start) == false)//选择最小的位置独立化
					return false;

				std::vector<node_pos_get> pos_seek(
					{ 
						path_one.pos_start, path_one.pos_end,
					    path_sec.pos_start, path_sec.pos_end,
					    path_one.pos_end + 1, path_sec.pos_end + 1
					}
				);
				std::vector<node_pos> pos_save;

				std::sort(pos_seek.begin(), pos_seek.end(), [](node_pos_get pos_one, node_pos_get pos_sec)->bool {return pos_one < pos_sec; }); //升序

				if (get_node_pos_by_node_pos_get_mul(path_info_target, std::move(pos_seek), pos_save) == false)
					return false;

				for (std::vector<node_pos>::iterator pos = pos_save.begin(); pos != pos_save.end() - 1; pos++)
				{
					if (test_validity_node_pos(*pos) == false)
						return false;
				}

				ahead* end_last = get_ahead_by_node_pos(pos_save[end_last_index]);
				ahead* end_tail = get_ahead_by_node_pos(pos_save[end_tail_index]);
				ahead* start_last = get_ahead_by_node_pos(pos_save[start_last_index]);
				ahead* start_tail = get_ahead_by_node_pos(pos_save[start_tail_index]);

				if (end_last == nullptr) path_info_target->info.last_pos = pos_save[start_index];
				else end_last->last = pos_save[start_index];

				if (pos_save[end_tail_index] != pos_save[start_last_index])
				{
					node_pos node_temp = start_tail->last;

					start_tail->last = end_tail->last;
					end_tail->last = node_temp;
					start_last->last = pos_save[end_index];
				}
				else
				{
					end_tail->last = start_tail->last;
					start_tail->last = pos_save[end_index];
				}

				node_pos_get pos_get_mid_new = static_cast<node_pos_get>(path_info_target->info.len / 2);
				size_t len_loop = pos_get_mid_new == 0 ? 1 : static_cast<size_t>(pos_get_mid_new);
				node_pos pos_mid = path_info_target->info.last_pos;

				while (len_loop > 0)
				{
					pos_mid = get_ahead_by_node_pos(pos_mid)->last;
					len_loop--;
				}

				path_info_target->info.mid_pos = pos_mid;
			}
			else
			{
				path_info* path_info_one = get_path_info_by_path_pos_ptr(get_path_pos(path_one.index));
				path_info* path_info_sec = get_path_info_by_path_pos_ptr(get_path_pos(path_sec.index));

				if (path_info_one == nullptr)
					return false;
				if (path_info_sec == nullptr)
					return false;

				if (make_path_unique_part(path_info_one, path_one.pos_start) == false)
					return false;
				if (make_path_unique_part(path_info_sec, path_sec.pos_start) == false)
					return false;

				std::vector<node_pos> pos_save_one;
				std::vector<node_pos> pos_save_sec;

				if (get_node_pos_by_node_pos_get_mul(path_info_one, std::move(std::vector<node_pos_get>({ path_one.pos_start, path_one.pos_end, path_one.pos_end + 1 })), pos_save_one) == false)
					return false;
				if (get_node_pos_by_node_pos_get_mul(path_info_sec, std::move(std::vector<node_pos_get>({ path_sec.pos_start, path_sec.pos_end, path_sec.pos_end + 1 })), pos_save_sec) == false)
					return false;

				if (test_validity_node_pos(pos_save_one[start_index]) == false)
					return false;
				if (test_validity_node_pos(pos_save_one[start_tail_index]) == false)
					return false;
				if (test_validity_node_pos(pos_save_sec[start_index]) == false)
					return false;
				if (test_validity_node_pos(pos_save_sec[start_tail_index]) == false)
					return false;

				ahead* one_end_last = get_ahead_by_node_pos(pos_save_one[start_last_index]);
				ahead* one_end_tail = get_ahead_by_node_pos(pos_save_one[start_tail_index]);
				ahead* sec_end_last = get_ahead_by_node_pos(pos_save_sec[start_last_index]);
				ahead* sec_end_tail = get_ahead_by_node_pos(pos_save_sec[start_tail_index]);

				data_type_base_signed len_delta =
					static_cast<data_type_base_signed>(path_one.pos_end - path_one.pos_start + 1) -
					static_cast<data_type_base_signed>(path_sec.pos_end - path_sec.pos_start + 1);

				if (one_end_last == nullptr)
					path_info_one->info.last_pos = pos_save_sec[start_index];
				else
					one_end_last->last = pos_save_sec[start_index];

				if (sec_end_last == nullptr)
					path_info_sec->info.last_pos = pos_save_one[start_index];
				else
					sec_end_last->last = pos_save_one[start_index];

				node_pos node_temp = one_end_tail->last;
				one_end_tail->last = sec_end_tail->last;
				sec_end_tail->last = node_temp;

				path_info_one->info.len =
					len_delta < 0 ?
					path_info_one->info.len + static_cast<path_len>(-len_delta) :
					path_info_one->info.len - static_cast<path_len>(len_delta);

				size_t len_loop = static_cast<size_t>(path_info_one->info.len / 2);
				node_pos pos_mid = path_info_one->info.last_pos;

				while (len_loop > 0)
				{
					pos_mid = get_ahead_by_node_pos(pos_mid)->last;
					len_loop--;
				}

				path_info_one->info.mid_pos = pos_mid;

				path_info_sec->info.len =
					len_delta > 0 ?
					path_info_sec->info.len + static_cast<path_len>(len_delta) :
					path_info_sec->info.len - static_cast<path_len>(-len_delta);

				len_loop = static_cast<size_t>(path_info_sec->info.len / 2);
				pos_mid = path_info_sec->info.last_pos;

				while (len_loop > 0)
				{
					pos_mid = get_ahead_by_node_pos(pos_mid)->last;
					len_loop--;
				}

				path_info_sec->info.mid_pos = pos_mid;
			}

			return true;
		}
		//修改路径中指定位置的节点编号
		bool change_path_node(path_index path, node_pos_get pos, node_index index) noexcept
		{
			path_info* path_info_target = get_path_info_by_path_pos_ptr(get_path_pos(path));

			if (path_info_target == nullptr)
				return false;//无效的路径

			if (make_path_unique_part(path_info_target, pos) == false)
				return false;//独立化失败

			ahead* ahead_target = get_ahead_by_node_pos(get_node_pos_by_node_pos_get(path_info_target, pos));

			if (ahead_target == nullptr)
				return false;

			ahead_target->index = index;
			return true;
		}
		//批量删除路径中的节点编号
		bool del_path_node(path_index path, std::vector<node_pos_get>& del_node)
		{
			if (del_node.empty() == true)
				return true;//空的请求

			path_pos path_pos_target = get_path_pos(path);
			path_info* path_info_target = get_path_info_by_path_pos_ptr(path_pos_target);

			if (path_info_target == nullptr)
				return false;

			std::sort(del_node.begin(), del_node.end(), [](node_pos_get pos_one, node_pos_get pos_sec)->bool {return pos_one < pos_sec; });//升序
			del_node.erase(std::unique(del_node.begin(), del_node.end()), del_node.end());

			size_t len_del = del_node.size();
			node_pos_get node_last_del = del_node.back();

			if (node_last_del >= static_cast<node_pos_get>(path_info_target->info.len))
				return false;//存在超出范围的请求

			if (make_path_unique_part(path_info_target, del_node.front()) == false)
				return false;//独立化失败

			std::vector<node_pos> save;

			if (get_node_pos_by_node_pos_get_mul(path_info_target, std::move(std::vector<node_pos_get>({ node_last_del,node_last_del + 1 })), save) == false)
				return false;

			node_pos pos_last = save.back();
			node_pos pos_current = save.front();
			node_pos pos_info = path_info_target->info.last_pos;

			size_t len_loop = static_cast<size_t>(node_last_del - del_node.front());
			size_t len_current = 0;
			node_pos_get pos_current_get = node_last_del;

			//开始删除并重连前驱
			while (len_current <= len_loop)
			{
				ahead* node_last = get_ahead_by_node_pos(pos_last);
				ahead* node_current = get_ahead_by_node_pos(pos_current);

				if (pos_current_get == del_node.back())
				{
					node_pos pos_last_temp = node_current->last;

					node_current->ref--;

					if (node_last != nullptr)
						node_last->last = pos_last_temp;

					del_ahead_by_node_pos(pos_current);

					if (pos_info == pos_current)
						pos_info = pos_last_temp;

					pos_current = pos_last_temp;
					del_node.pop_back();
				}
				else
				{
					pos_last = pos_current;
					pos_current = node_current->last;
				}

				pos_current_get--;
				len_current++;
			}

			path_len len_new = path_info_target->info.len - static_cast<path_len>(len_del);

			if (len_new == 0)
			{
				del_path_info_by_path_pos(path_pos_target);
				return true;
			}

			len_loop = static_cast<size_t>(len_new / 2);
			node_pos pos_mid = pos_info;

			//更新中点位置
			while (len_loop > 0)
			{
				pos_mid = get_ahead_by_node_pos(pos_mid)->last;
				len_loop--;
			}

			path_info_target->info = path_info_base(pos_info, pos_mid, len_new);
			return true;
		}
		//删除路径中指定位置的节点编号
		bool del_path_node_single(path_index path, node_pos_get del_node)
		{
			std::vector<node_pos_get> del_target({ del_node });
			return del_path_node(path, del_target);//单节点删除
		}
		//删除指定路径
		bool del_path(path_index path)
		{
			path_pos pos_target = get_path_pos(path);

			if (test_validity_path_pos(pos_target) == false)
				return false;

			path_info* path_target = get_path_info_by_path_pos_ptr(pos_target);

			if (path_target == nullptr)
				return false;//无效的路径

			node_pos pos_last = NODE_INVALID_POS;
			node_pos pos_current = path_target->info.last_pos;
			ahead* node_current = get_ahead_by_node_pos(pos_current);
			size_t len_loop = static_cast<size_t>(path_target->info.len);
			size_t len_current = 0;

			//开始逐个删除前驱
			while (len_current < len_loop)
			{
				pos_last = pos_current;
				pos_current = node_current->last;

				if (--node_current->ref == 0)
					del_ahead_by_node_pos(pos_last);//已没有额外引用，直接删除

				node_current = get_ahead_by_node_pos(pos_current);
				len_current++;
			}

			del_path_info_by_path_pos(pos_target);
			return true;
		}
		//清空整个共享系统
		void clear()
		{
			//清空系统
			clear_all();
			return;
		}
	public:
		//统计系统信息
		path_state_statistic get_stastic() const noexcept
		{
			return std::move(get_state());
		}
	public:
		//获取路径指定位置的节点编号
		node_index operator()(path_index path, node_pos_get pos) noexcept
		{
			return get_path_node_index(path, pos);
		}
		//修改路径中指定位置的节点编号
		bool operator()(path_index path, node_pos_get pos, node_index index) noexcept
		{
			return change_path_node(path, pos, index);
		}
		//获取路径
		bool operator()(path_index path_index, path& save) const
		{
			return get_path(path_index, save);
		}
		//...
		path_share& operator=(const path_share& other)
		{
			if (this != &other)
			{
				this->~path_share();
				new (this) path_share(other);
			}
			return *this;
		}
		//...
		path_share& operator=(path_share&& other) noexcept
		{
			if (this != &other)
			{
				this->node_set = std::move(other.node_set);
				this->node_set_free = std::move(other.node_set_free);
				this->path_set = std::move(other.path_set);
				this->path_index_free = std::move(other.path_index_free);
			}
			return *this;
		}
	};





}

