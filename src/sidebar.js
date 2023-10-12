


const tocContainer = document.querySelector("#toc");
const tocTitle = document.createElement("div");
tocTitle.innerHTML = "目次";
tocTitle.classList.add("toc_title");
tocContainer.appendChild(tocTitle);



page_contents.forEach(item => {
    if (item.type === "H2" || item.type === "H3") {
        const listItem = document.createElement("li");
        listItem.innerHTML = `<a href="#${item.id}">${item.title}</a>`;
        listItem.classList.add("toc_" + item.type);
        tocContainer.appendChild(listItem);
    }
});

const options = {
    root: null,
    rootMargin: "-50% 0px",
    threshold: 0
};

const observer = new IntersectionObserver(onIntersection, options);


page_contents.forEach(item => {
    const element = document.getElementById(item.id);
    if (element) {
        observer.observe(element);
    }
});

prev_item = null;

function onIntersection(entries) {
    entries.forEach(entry => {
        const id = entry.target.id;
        const tocItem = document.querySelector(`#toc a[href="#${id}"]`);
        if (tocItem) {
            if (entry.isIntersecting) {
                tocItem.classList.add("active");
                if (prev_item) {
                    prev_item.classList.remove("active");
                }
                prev_item = tocItem;
            }
        }
    });
}



