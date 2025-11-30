import { useRef } from "react";
import "../../styles/landingPage.css";
import RegisterLogin from "./RegisterLogin";

const LandingPage = () => {
    const mainDisplayRef = useRef(null);

    return (
        <div ref={mainDisplayRef} id="landingPage__cont">
            <div className="landingPage__title">
                <h1>C++ HTTP Server</h1>
                <p>✨ Note: This is not a Web-development project! The backend C++ HTTP Server built from scratch is the main project.✨</p>
            </div>
            <RegisterLogin />
        </div>
    );
};

export default LandingPage;
