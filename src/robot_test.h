/*  Copyright (c) 2014 "Kaazing Corporation," (www.kaazing.com)
**
**  This file is part of Robot.
**
**  Robot is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Affero General Public License as
**  published by the Free Software Foundation, either version 3 of the
**  License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  You should have received a copy of the GNU Affero General Public License
**  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ROBOT_TEST_H_
#define ROBOT_TEST_H_

typedef struct {
	char * actual_script;
	char * expected_script;
} result;

/*
**  Arguments:
**	W (absolute path of the script e.g. /home/user/../script_name.rpt),
**	X (function where your client code is, NULL if none),
**	Y (function to any clean-up code you need to run after the client code, NULL if none)
**	Z (in seconds, set <= 0 for no timeout)
*/
#define ROBOT_TEST(W, X, Y, Z) robotTestWrapper((char *)W, (void *)X, (void *)Y, (int)Z)

extern "C" {
	/*
	** 
	** Send the command to start the robot (which will result in robot attempts to connect if script contains connect)
	**
	*/
	void ROBOT_JOIN(void);
	
	/*
	** Executes the given client code against the provided robot script and
	** returns the expected script and actual script in a result structure.
	** Returns NULL if error occurs
	** NOTE: my_name is set here
	**
	** Arguments:
	** abs_path - the absolute path of the script (e.g. /home/user/../script_name.rpt),
	** func - function pointer function where your client code is, NULL if none),
	** seconds - timeout (set <= 0 for no timeout)
	*/
	result * robotTest(char * abs_path, void * func, int seconds);
}
/*	
**  This provides a c++ wrapper for executing a robot test through the Google Test testing framework
**  Executes the given client code against the provided robot script and asserts that the
**  resulting script matches the expected script. It also cleans up the memory allocated for the results
**  and executes the given clean-up function if provided.
**  NOTE: remember to start the robot before executing any tests
**
**  Arguments: 
**	abs_path - the absolute path of the script (e.g. /home/user/../script_name.rpt),
**	functionPointer (function where your client code is, NULL if none), 
**	functionPointer (function to any clean-up code you need to run after the client code, NULL if none) 
**	timeout (in seconds, set <= 0 for no timeout) 
*/
void robotTestWrapper(char * abs_path, void * function, void * cleanup, int timeout);

#endif
