CREATE TABLE allowed_in_mail (
    email character varying(64) NOT NULL
);
CREATE TABLE push_ids (
    email character varying(64) NOT NULL,
    push_id character varying(256) NOT NULL,
    push_type integer DEFAULT 0 NOT NULL
);
ALTER TABLE ONLY allowed_in_mail
    ADD CONSTRAINT allowed_in_mail_pkey PRIMARY KEY (email);
ALTER TABLE ONLY push_ids
    ADD CONSTRAINT push_ids_pkey PRIMARY KEY (email, push_id);