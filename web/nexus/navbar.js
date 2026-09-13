const navbar = document.querySelector("body > nav"), navbarContent = document.querySelector("body > nav > div.content");

const navExit = () => {
    navbar.classList.remove("hovering");
};

const navEnter = (page) => {
    navbar.classList.add("hovering");
    //TODO add some code to change the content of the navbar based on the page param
};