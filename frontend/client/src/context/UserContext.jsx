import axios from "axios";
import React, { createContext, useState, useEffect, useMemo } from "react";

export const UserContext = createContext();

export const UserProvider = (props) => {
    const [loggedUser, setLoggedUser] = useState(null);

    // Force no redirects, when checking if the logged users token is valid.
    const [isLoading, setIsLoading] = useState(true);

    useEffect(() => {
        const checkSessionToken = async () => {
            try {
                const res = await axios.get(
                    "http://localhost:8080/api/checkUserSession",
                    {
                        withCredentials: true, // send cookies
                    }
                );

                if (res.status === 200) {
                    console.log("User session restored via Cookie.");
                    const userData = {
                        userID: res.data.userID,
                        username: res.data.username,
                    };
                    setLoggedUser(userData); // this triggers the re-render and useEffect above will run
                }
            } catch (err) {
                console.log("Session token cookie not valid");
                setLoggedUser(null);
            } finally {
                setIsLoading(false);
            }
        };
        checkSessionToken();
    }, []);

    const userValue = useMemo(
        () => ({ loggedUser, setLoggedUser }),
        [loggedUser, setLoggedUser]
    );

    return (
        <UserContext.Provider value={userValue}>
            {!isLoading && props.children}
        </UserContext.Provider>
    );
};
