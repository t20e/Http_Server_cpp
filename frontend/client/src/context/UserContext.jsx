import React, { createContext, useState, useMemo } from "react";

export const UserContext = createContext();

export const UserProvider = (props) => {
    const [loggedUser, setLoggedUser] = useState(null);
    const userValue = useMemo(
        () => ({ loggedUser, setLoggedUser }),
        [loggedUser, setLoggedUser]
    );

    return (
        <UserContext.Provider value={userValue}>
            {props.children}
        </UserContext.Provider>
    );
};
