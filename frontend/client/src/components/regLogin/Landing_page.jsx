import { useState, useRef } from "react";
import "../../styles/landingPage.css";
import RegisterLogin from "./RegisterLogin";

const Landing_page = () => {
    const mainDisplayRef = useRef(null);
    const [vibrateErr, setVibrateErr] = useState("");

    const changeErrVibrate = () => {
        setVibrateErr("errVibrate");
        setTimeout(() => {
            setVibrateErr("");
        }, 500);
    };

    return (
        <div ref={mainDisplayRef} id="landingPage__cont">
            <div className="landingPage__title">
                <h1>C++ HTTP Server</h1>
                <p>Note: This is not a Web-development project!</p>
            </div>
            <RegisterLogin
                vibrateErr={vibrateErr}
                changeErrVibrate={changeErrVibrate}
            />
        </div>
    );
};

export default Landing_page;
