=======================================================
h1 Recoverable Virtual Memory: AOS Spring 2015 Project 4
=======================================================

h4 1. Team:
  * **Harshit Jain**		:903024992
  * **Dibakar Barua**		:903061468

h4 2. Logfiles are written on a rvm_commit_trans(). They are written in this format:
	* *<offset>~~::separator::~~<data>*

h4 3. For persistency undo log and redo log are created in virtual memory. Undo log copys its data to virtual memory on an abort and redo log commits its data as *<segment_name>.logfile* on a commit in the above format.

h4 4. On a rvm_truncate_log() the library truncates the respective logfiles for the transaction and modifes the segment files.

h4 5. logiles are cleaned up on a  rvm_truncate_log() or if one tries to do a rvm_map() on an existing untruncated segment. Calling a rvm_map on an existing untruncated segment results in truncation of the logs belonging to that segment.

h4 6. Once created logfiles are indefinitely allowed to expand until it is truncated to the segment.
 
