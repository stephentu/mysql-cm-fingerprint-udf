This is a simple aggregate UDF for MySQL which computes the CM-fingerprint of a aggregate group and returns the fingerprint table as a binary string. Has only been tested on MySQL 5.5. You can install the function by running the following command as an admin:

  CREATE AGGREGATE FUNCTION cm_fingerprint RETURNS STRING SONAME 'libcmfp.so';
