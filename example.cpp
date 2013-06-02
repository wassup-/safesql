#include "safesql.hpp"

#include <iostream>

using fp::sql::_;
using fp::sql::query;

int main(int argc, char ** argv) {
	using std::to_string;

	query qry;
	qry << "SELECT p_name, p_city ";
	qry << "FROM tbl_people ";
	qry << "WHERE p_id = " << _;
	qry << " OR p_name LIKE " << _;
	qry << " AND p_age < " << _;
	std::cout << "Original query: " << to_string(qry) << std::endl;
	std::cout << "Executed query: " << qry.execute(5, "%ket% %jap%", 20) << std::endl;
	return 0;
}