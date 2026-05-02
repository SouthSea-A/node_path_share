#include "../node_path_share.h"

#include <cassert>
#include <vector>

using namespace node_path;

static std::vector<node_index> read_path(path_share& paths, path_index index)
{
	path result;
	assert(paths.get_path(index, result));
	return result.nodes;
}

int main()
{
	path_share paths;

	path_index main_path = paths.create_path(1);
	assert(main_path != PATH_INVALID_INDEX);
	assert(paths.add_path_node(main_path, std::vector<node_index>({ 2, 3, 4, 5 })));

	path_index branch_one = paths.create_branch(main_path, 2, 6);
	assert(branch_one != PATH_INVALID_INDEX);
	path_index branch_two = paths.create_branch(main_path, 3, 7);
	assert(branch_two != PATH_INVALID_INDEX);

	path_index same_one = paths.create_path(10);
	assert(same_one != PATH_INVALID_INDEX);
	assert(paths.add_path_node(same_one, std::vector<node_index>({ 11, 12 })));

	path_index same_two = paths.create_path(10);
	assert(same_two != PATH_INVALID_INDEX);
	assert(paths.add_path_node(same_two, std::vector<node_index>({ 11, 12 })));

	path_index sparse_path = paths.create_path(20);
	assert(sparse_path != PATH_INVALID_INDEX);
	assert(paths.add_path_node(sparse_path, std::vector<node_index>({ 21, 22 })));
	assert(paths.del_path_node_single(sparse_path, 1));

	const std::vector<node_index> main_before = read_path(paths, main_path);
	const std::vector<node_index> branch_one_before = read_path(paths, branch_one);
	const std::vector<node_index> branch_two_before = read_path(paths, branch_two);
	const std::vector<node_index> same_one_before = read_path(paths, same_one);
	const std::vector<node_index> same_two_before = read_path(paths, same_two);
	const std::vector<node_index> sparse_before = read_path(paths, sparse_path);

	const path_state_statistic before = paths.get_stastic();
	assert(before.quantity_bytes_memory_alloc > before.quantity_bytes_memory_used);

	assert(paths.unify());

	const path_state_statistic after = paths.get_stastic();
	assert(after.quantity_bytes_memory_alloc == after.quantity_bytes_memory_used);
	assert(after.quantity_bytes_memory_alloc < before.quantity_bytes_memory_alloc);

	assert(read_path(paths, main_path) == main_before);
	assert(read_path(paths, branch_one) == branch_one_before);
	assert(read_path(paths, branch_two) == branch_two_before);
	assert(read_path(paths, same_one) == same_one_before);
	assert(read_path(paths, same_two) == same_two_before);
	assert(read_path(paths, sparse_path) == sparse_before);

	assert(paths.get_path_node_index(main_path, 2) == main_before[2]);
	assert(paths.get_path_node_index(branch_one, 1) == branch_one_before[1]);
	assert(paths.get_path_node_index(branch_two, 3) == branch_two_before[3]);
	assert(paths.get_path_node_index(sparse_path, 1) == sparse_before[1]);

	return 0;
}
