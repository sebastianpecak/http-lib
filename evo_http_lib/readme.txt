
/////////////////////////////////////////////////////////////////////////////

To start using HttpLib you have to make sure that your application is initialized with EESL_InitializeEx function.
It is because underlying VCSLib uses 500B buffer for data exchange with VCS task.
By using EESL_InitializeEx function we allow app using EESL buffers exceeding over 300B.

/////////////////////////////////////////////////////////////////////////////