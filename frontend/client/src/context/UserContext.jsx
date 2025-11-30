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
                    "http://localhost:8080/api/getLoggedUser",
                    {
                        withCredentials: true, // Send cookie
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
                if (err.status === 403) {
                    alert(
                        "Origin is not allowed, please use: http://localhost:3000/"
                    );
                } else if (err.status === 401) {
                    console.log("Session token cookie not valid. Err: ", err);
                    setLoggedUser(null);
                }
            } finally {
                setIsLoading(false);
            }
        };
        checkSessionToken();
    }, []);

    const userValue = useMemo(
        () => ({ loggedUser, setLoggedUser, isLoading }),
        [loggedUser, setLoggedUser, isLoading]
    );

    return (
        <UserContext.Provider value={userValue}>
            {isLoading ? (
                <div className="text-center py-5">
                    <p>Loading session...</p>
                </div>
            ) : (
                props.children
            )}
        </UserContext.Provider>
    );
};
